#include "MDLExporter.h"
#include "../ace_cpp/common/Math/ace.Vector2DF.h"
#include "../ace_cpp/common/Math/ace.Vector3DF.h"
#include "../ace_cpp/common/Math/ace.Matrix44.h"

#include <iostream>
#include <algorithm>

MDLExporter::MDLExporter(const char* fileName){
	// Initialize the SDK manager. This object handles all our memory management.
	m_SdkManager = FbxManager::Create();

	// Create the IO settings object.
	FbxIOSettings *ios = FbxIOSettings::Create(m_SdkManager, IOSROOT);
	m_SdkManager->SetIOSettings(ios);

	// Create an importer using the SDK manager.

	fbxsdk_2014_2_1::FbxImporter* lImporter = fbxsdk_2014_2_1::FbxImporter::Create(m_SdkManager, "");

	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(fileName, -1, m_SdkManager->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		system("PAUSE");
		exit(-1);
	}

	// Create a new scene so that it can be populated by the imported file.
	m_Scene = FbxScene::Create(m_SdkManager, "myScene");

	// Import the contents of the file into the scene.
	lImporter->Import(m_Scene);

	// The file is imported; so get rid of the importer.
	lImporter->Destroy();


	m_binaryWriter = new ace::BinaryWriter();

}

void MDLExporter::Convert()
{
	PrintHeader();
	FbxNode* lRootNode = m_Scene->GetRootNode();
	if (lRootNode) {

		//ボーンリスト
		{
			std::vector<int> armatureIndices;
			for(int i=0;i<lRootNode->GetChildCount();++i)
			{
				if(lRootNode->GetChild(i)->GetNodeAttribute()->GetAttributeType()==FbxNodeAttribute::eSkeleton)
				{
					armatureIndices.push_back(i);
				}
			}

			m_meshGroups.resize((int32_t)armatureIndices.size());

			int armaturePtr=0;
			for (int i = 0; i < (int)armatureIndices.size(); i++)
			{
				Deformer::ResetIndexCount();
				GetDeformer(NULL,lRootNode->GetChild(armatureIndices[i]),m_meshGroups[armaturePtr++].deformerManager);
			}
		}

		//アニメーション
		{
			const int stackCount = m_Scene->GetSrcObjectCount<FbxAnimStack>();

			for(int i=0;i<stackCount;++i)
			{
				AnimationSource animationSource;
				FbxAnimStack *pStack = m_Scene->GetSrcObject<FbxAnimStack>(i);

				m_Scene->SetCurrentAnimationStack(pStack);
				AnimStackAnalyze(pStack,lRootNode,animationSource);

				m_animationSources.push_back(animationSource);
			}
		}

		//メッシュ一覧
		{
			for (int i = 0; i < lRootNode->GetChildCount(); i++)
				GetMeshGroup(lRootNode->GetChild(i),0);
		}
	}


	//出力
	{
		int meshGroupNum = (int) m_meshGroups.size();

		printf("Mesh Group Num = %d\n", meshGroupNum);

		//メッシュ
		m_binaryWriter->Push(meshGroupNum);

		for (int i = 0; i < meshGroupNum; ++i)
		{
			MeshGroup meshGroup=m_meshGroups[i];

			printf("Mesh Num = %d\n", meshGroup.meshLoaders.size());

			m_binaryWriter->Push((int32_t)meshGroup.meshLoaders.size());
			for(int j=0;j<meshGroup.meshLoaders.size();++j)
			{
				meshGroup.meshLoaders[j].WriteVertices(m_binaryWriter);
				meshGroup.meshLoaders[j].WriteFaces(m_binaryWriter);
				meshGroup.meshLoaders[j].WriteFaceMaterials(m_binaryWriter);
				meshGroup.meshLoaders[j].WriteBoneAttachments(m_binaryWriter);
			}
			//ボーン
			meshGroup.deformerManager.WriteDeformerInformation(m_binaryWriter);

			//材質
			meshGroup.WriteMaterials(m_binaryWriter);
		}
	}

	{
		//アニメーションソース
		m_binaryWriter->Push((int32_t)m_animationSources.size());
		for(int i=0;i<m_animationSources.size();++i)
		{
			m_animationSources[i].WriteAnimationSource(m_binaryWriter);
		}
	}

	{
		//アニメーションクリップ
		m_binaryWriter->Push((int32_t)m_animationSources.size());
		for(int i=0;i<m_animationSources.size();++i)
		{
			m_binaryWriter->Push(ace::ToAString(m_animationSources[i].animationName.c_str()));
			m_binaryWriter->Push(i);
		}
	}


	m_binaryWriter->WriteOut("out.mdl");

}

MDLExporter::~MDLExporter()
{
	m_SdkManager->Destroy();

	delete m_binaryWriter;
}

void MDLExporter::GetMeshGroup(FbxNode* pNode,int depth)
{
	// Print the node's attributes.

	GetMeshProperty(pNode);

	for (int j = 0; j < pNode->GetChildCount(); j++)
		GetMeshGroup(pNode->GetChild(j),depth+1);
}

void MDLExporter::GetMeshProperty(FbxNode* node)
{
	if(node->GetNodeAttribute()->GetAttributeType()!=FbxNodeAttribute::eMesh) return;

	for (int i = 0; i < node->GetNodeAttributeCount(); ++i)
	{
		FbxNodeAttribute *pAttribute = node->GetNodeAttributeByIndex(i);
		FbxMesh* mesh = (FbxMesh*) pAttribute;

		if (!mesh->IsTriangleMesh())
		{
			FbxGeometryConverter _converter(m_SdkManager);
			mesh = (FbxMesh*) _converter.Triangulate(mesh, true);
		}

		MeshLoader mLoader;

		int attachmentIndex;

		mLoader.Load(mesh,attachmentIndex,m_meshGroups);

		if(attachmentIndex!=-1)
		{
			for(int j=0;j<mLoader.materials.size();++j)
			{
				/*
				mLoader.materials[j].groupIndex=_meshGroups[attachmentIndex].materials.size();
				if(std::find(_meshGroups[attachmentIndex].materials.begin(),_meshGroups[attachmentIndex].materials.end(),mLoader.materials[j])==_meshGroups[attachmentIndex].materials.end())
				{
					_meshGroups[attachmentIndex].materials.push_back(mLoader.materials[j]);
				}
				*/
				mLoader.materials[j].groupIndex=m_meshGroups[attachmentIndex].materials.size();
				m_meshGroups[attachmentIndex].materials.push_back(mLoader.materials[j]);

			}
			m_meshGroups[attachmentIndex].meshLoaders.push_back(mLoader);
		}
		else
		{
			MeshGroup meshGroup;

			for(int j=0;j<mLoader.materials.size();++j)
			{
				mLoader.materials[j].groupIndex=j;
				meshGroup.materials.push_back(mLoader.materials[j]);
			}

			meshGroup.meshLoaders.push_back(mLoader);
			m_meshGroups.push_back(meshGroup);
		}
	}

}

void MDLExporter::GetDeformer(Deformer* parentSkeleton, FbxNode* pNode,DeformerManager &deformerManager)
{
	Deformer *deformer;
	deformer=new Deformer();
	GetDeformerProperty(parentSkeleton,pNode,deformer,deformerManager);

	deformerManager.AddDeformer(deformer);

	for(int j=0;j<pNode->GetChildCount();++j)
	{
		GetDeformer(deformer,pNode->GetChild(j),deformerManager);
	}
}

void MDLExporter::AnimStackAnalyze(FbxAnimStack* pStack, FbxNode *rootNode,AnimationSource &animationSource)
{
	const int layerCount = pStack->GetMemberCount<FbxAnimLayer>();

	if(layerCount>0)
	{
		const char* animationName = pStack->GetName();

		FbxTime startTime = pStack->LocalStart;
		FbxTime endTime = pStack->LocalStop;

		animationSource.animationName=std::string(animationName);
		animationSource.startTime=startTime;
		animationSource.stopTime=endTime;
	}

	for(int i=0;i<layerCount;++i)
	{
		FbxAnimLayer *pLayer = pStack->GetMember<FbxAnimLayer>();
		GetMotion(NULL,rootNode,pLayer,animationSource);
		break;
	}
}

void MDLExporter::GetMotion(FbxNode *parentNode,FbxNode* node,FbxAnimLayer* pLayer,AnimationSource &animationSource)
{
	//assert(node->GetNodeAttribute());

	if(node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType()==FbxNodeAttribute::eSkeleton)
	{
		GetSkeletonCurve(node,pLayer,animationSource);
	}

	for(int i=0;i<node->GetChildCount();++i)
	{
		GetMotion(node,node->GetChild(i),pLayer,animationSource);
	}
}

void MDLExporter::GetSkeletonCurve(FbxNode* fbxNode,FbxAnimLayer* fbxAnimLayer,AnimationSource &animationSource)
{
	std::string boneName = fbxNode->GetName();

	//Blenderのみz,yの変形を入れ替える
	AnalyzeCurve(boneName+".pos.x",fbxNode->LclTranslation.GetCurve(fbxAnimLayer,FBXSDK_CURVENODE_COMPONENT_X),animationSource);
	AnalyzeCurve(boneName+".pos.z",fbxNode->LclTranslation.GetCurve(fbxAnimLayer,FBXSDK_CURVENODE_COMPONENT_Y),animationSource);
	AnalyzeCurve(boneName+".pos.y",fbxNode->LclTranslation.GetCurve(fbxAnimLayer,FBXSDK_CURVENODE_COMPONENT_Z),animationSource);

	AnalyzeCurve(boneName+".rot.x",fbxNode->LclRotation.GetCurve(fbxAnimLayer,FBXSDK_CURVENODE_COMPONENT_X),animationSource);
	AnalyzeCurve(boneName+".rot.z",fbxNode->LclRotation.GetCurve(fbxAnimLayer,FBXSDK_CURVENODE_COMPONENT_Y),animationSource);
	AnalyzeCurve(boneName+".rot.y",fbxNode->LclRotation.GetCurve(fbxAnimLayer,FBXSDK_CURVENODE_COMPONENT_Z),animationSource);

	AnalyzeCurve(boneName+".scl.x",fbxNode->LclScaling.GetCurve(fbxAnimLayer,FBXSDK_CURVENODE_COMPONENT_X),animationSource);
	AnalyzeCurve(boneName+".scl.z",fbxNode->LclScaling.GetCurve(fbxAnimLayer,FBXSDK_CURVENODE_COMPONENT_Y),animationSource);
	AnalyzeCurve(boneName+".scl.y",fbxNode->LclScaling.GetCurve(fbxAnimLayer,FBXSDK_CURVENODE_COMPONENT_Z),animationSource);
}

void MDLExporter::AnalyzeCurve(std::string target,FbxAnimCurve* pCurve,AnimationSource &animationSource)
{
	KeyFrameAnimation keyFrameAnimation;
	keyFrameAnimation.targetName=target;

	const int keyCount = pCurve->KeyGetCount();

	int hour,minute,second,frame,field,residual;

	for(int i=0;i<keyCount;++i)
	{
		float value = pCurve->KeyGetValue(i);
		FbxTime time = pCurve->KeyGetTime(i);
		auto interpolation = pCurve->KeyGetInterpolation(i);
		time.GetTime(hour,minute,second,frame,field,residual,FbxTime::eFrames60);

		KeyFrame keyFrame;
		keyFrame.keyValue=ace::Vector2DF(60*(hour*60*60+minute*60+second+(float)frame/60),value);
		keyFrame.leftPosition=keyFrame.keyValue;
		keyFrame.rightPosition=keyFrame.keyValue;

		switch(interpolation)
		{
		case fbxsdk_2014_2_1::FbxAnimCurveDef::eInterpolationConstant:
			{
				keyFrame.interpolation=1;
			}
			break;
		case fbxsdk_2014_2_1::FbxAnimCurveDef::eInterpolationLinear:
			{
				keyFrame.interpolation=2;
			}
			break;
		case fbxsdk_2014_2_1::FbxAnimCurveDef::eInterpolationCubic:
			{
				keyFrame.interpolation=3;
			}
			break;
		}

		keyFrameAnimation.keyFrames.push_back(keyFrame);
	}
	animationSource.keyFrameAnimations.push_back(keyFrameAnimation);
}

void MDLExporter::GetDeformerProperty(Deformer* parentSkeleton, FbxNode* node,Deformer *deformer, DeformerManager &deformerManager)
{
	if(node->GetNodeAttribute()->GetAttributeType()!=FbxNodeAttribute::eSkeleton) return;


	for (int i = 0; i < node->GetNodeAttributeCount(); ++i)
	{
		FbxSkeleton *sk = (FbxSkeleton*) node->GetNodeAttributeByIndex(i);
		deformer->name = std::string(sk->GetNode()->GetName());

		deformer->parentIndex = (parentSkeleton == NULL) ? -1 : parentSkeleton->index;

		FbxAMatrix transform = node->EvaluateGlobalTransform();
		FbxAMatrix invMatrix = transform.Inverse();

		FbxAMatrix parentTransform;

		if(parentSkeleton == NULL)
		{
			parentTransform.SetIdentity();
		}
		else
		{
			parentTransform = node->GetParent()->EvaluateGlobalTransform();
		}

		FbxAMatrix invParentMatrix = parentTransform.Inverse();

		FbxAMatrix relationMatrix = invParentMatrix * transform;

		for(int j=0;j<4;++j)
		{
			for(int k=0;k<4;++k)
			{
				deformer->invMatrix.Values[j][k]=invMatrix.Get(k,j);
				deformer->relationMatrix.Values[j][k]=relationMatrix.Get(k,j);
			}
		}

		fbxsdk_2014_2_1::EFbxRotationOrder fbxRotationOrder;
		node->GetRotationOrder(FbxNode::eSourcePivot, fbxRotationOrder);

		switch(fbxRotationOrder)
		{
		case eEulerXYZ :
			deformer->rotationOrder=12;	break ;
		case eEulerXZY :
			deformer->rotationOrder=11;	break ;
		case eEulerYZX :
			deformer->rotationOrder=16;	break ;
		case eEulerYXZ :
			deformer->rotationOrder=15;	break ;
		case eEulerZXY :
			deformer->rotationOrder=13;	break ;
		case eEulerZYX :
			deformer->rotationOrder=14;	break ;
		case eSphericXYZ :
			break ;	
		}
	}
}

void MDLExporter::PrintHeader()
{
	m_binaryWriter->Push((uint8_t)'M');
	m_binaryWriter->Push((uint8_t)'D');
	m_binaryWriter->Push((uint8_t)'L');
	m_binaryWriter->Push((uint8_t) 0);

	m_binaryWriter->Push(1);
}