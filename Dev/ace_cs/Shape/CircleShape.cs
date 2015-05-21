﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ace
{
    public class CircleShape : Shape
    {
        internal override swig.CoreShape CoreShape
        {
            get { return coreCircle; }
        }
        private swig.CoreCircleShape coreCircle { get; set; }

        public override ShapeType ShapeType
        {
            get { return ShapeType.CircleShape; }
        }

        public CircleShape()
            : base()
        {
            coreCircle = Engine.ObjectSystemFactory.CreateCircleShape();

            var p = coreCircle.GetPtr();
            if (GC.Shapes.GetObject(p) != null)
            {
                throw new Exception();
            }
            GC.Shapes.AddObject(p, this);
        }

        #region GC対応
        ~CircleShape()
        {
            Destroy();
        }

        public override bool IsDestroyed
        {
            get { return coreCircle == null; }
        }

        public override void Destroy()
        {
            lock (this)
            {
                if (coreCircle == null) return;
                GC.Collector.AddObject(coreCircle);
                coreCircle = null;
            }
            System.GC.SuppressFinalize(this);
        }
        #endregion

        /// <summary>
        /// 弧の中心座標を取得または設定する。
        /// </summary>
        public Vector2DF Position
        {
            get
            {
                return coreCircle.GetPosition();
            }
            set
            {
                coreCircle.SetPosition(value);
            }
        }

        /// <summary>
        /// 弧の外径を取得または設定する。
        /// </summary>
        public float OuterDiameter
        {
            get
            {
                return coreCircle.GetOuterDiameter();
            }
            set
            {
                coreCircle.SetOuterDiameter(value);
            }
        }

        /// <summary>
        /// 弧の内径を取得または設定する。
        /// </summary>
        public float InnerDiameter
        {
            get
            {
                return coreCircle.GetInnerDiameter();
            }
            set
            {
                coreCircle.SetInnerDiameter(value);
            }
        }

        /// <summary>
        /// 弧の中心を軸とした回転角度を取得または設定する。
        /// </summary>
        public float Angle
        {
            get
            {
                return coreCircle.GetAngle();
            }
            set
            {
                coreCircle.SetAngle(value);
            }
        }

        /// <summary>
        /// 弧の外周、内周上の曲がり角（滑らかさ）の数を取得または設定する。
        /// </summary>
        public int NumberOfCorners
        {
            get
            {
                return coreCircle.GetNumberOfCorners();
            }
            set
            {
                coreCircle.SetNumberOfCorners(value);
            }
        }
    }
}