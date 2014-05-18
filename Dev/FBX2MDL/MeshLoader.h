#pragma once
#include <fbxsdk.h>
#include <vector>
#include "../ace_cpp/common/Utility/ace.BinaryWriter.h"
#include "../ace_cpp/common/Math/ace.Vector2DF.h"
#include "Mesh.h"
#include "Deformer.h"
#include "Material.h"
#define EPS 0.0001

class MeshLoader;

struct MeshGroup
{
	std::vector<MeshLoader> meshLoaders;
	DeformerManager deformerManager;
	std::vector<Material> materials;

	void WriteMaterials(ace::BinaryWriter* writer)
	{
		printf("Materials:%d\n",(int32_t)materials.size());
		writer->Push((int32_t)materials.size());
		for(auto ite=materials.begin();ite!=materials.end();++ite)
		{
			printf("index = %d\n",ite->groupIndex);
			writer->Push(ite->Type);
			for(int i=0;i<3;++i)
			{
				printf("mat:%s\n",ite->texture[i].c_str());
				writer->Push(ace::ToAString(ite->texture[i].c_str()));
			}
			printf("\n");
		}
	}
};

class MeshLoader
{
	std::string m_name;
	std::vector<Vertex> m_baseVertices;
	std::vector<Vertex> m_vertices;
	std::vector<Face> m_faces;
	std::vector<FacialMaterial> m_facialMaterials;

	int m_faceContinue;
	int m_preFaceIndex;


	void _loadPositions(FbxMesh* fbxMesh);

	ace::Vector3DF _loadNormal(FbxMesh* fbxMesh,int lControlPointIndex,int vertexId);
	ace::Vector3DF _loadBinormal(FbxMesh* fbxMesh,int lControlPointIndex,int vertexId);
	ace::Vector2DF _loadUV(FbxMesh* fbxMesh,int lControlPointIndex,int vertexId,int polygonCount,int polygonVert);
	std::vector<uint8_t> _loadColor(FbxMesh* fbxMesh,int lControlPointIndex,int vertexId);
	void _loadWeight(FbxMesh* fbxMesh,int& attachedIndex,std::vector<MeshGroup> &meshGroups);
	void _loadVertices(FbxMesh* fbxMesh);
	void _loadFaceIndices(FbxMesh* fbxMesh);
	void _loadFaceMaterials(FbxMesh* fbxMesh);
	void _loadBoneAttachments(FbxMesh* fbxMesh);

	//Material
	void _loadTextures(FbxMesh* fbxMesh);
public:
	MeshLoader();
	
	std::vector<Material> materials;
	std::vector<Vertex> GetVertices();
	std::vector<Face> GetFaces();

	void Load(FbxMesh* fbxMesh,int& attachmentIndex,std::vector<MeshGroup> &meshGroups);
	void WriteVertices(ace::BinaryWriter* writer);
	void WriteFaces(ace::BinaryWriter* writer);
	void WriteFaceMaterials(ace::BinaryWriter* writer);
	void WriteBoneAttachments(ace::BinaryWriter* writer);
	void WriteMaterials(ace::BinaryWriter* writer);

	MeshLoader& operator=( const MeshLoader &meshLoader)
	{
		m_baseVertices=meshLoader.m_baseVertices;
		m_vertices=meshLoader.m_vertices;
		m_faces=meshLoader.m_faces;
		//m_facialMaterials=meshLoader.m_facialMaterials;

		return *this;
	}

};