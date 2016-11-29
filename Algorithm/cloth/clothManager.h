#pragma once

#include <vector>
#include <memory>
#include "device_array.h"
#include "ldpMat\ldp_basic_vec.h"
#include <map>
//#define ENABLE_SELF_COLLISION
#ifdef ENABLE_SELF_COLLISION
#include "COLLISION_HANDLER.h"
#endif
class ObjMesh;
namespace svg
{
	class SvgManager;
	class SvgPolyPath;
}
namespace ldp
{
	struct SimulationParam
	{
		float rho;						// for chebshev accereration
		float under_relax;				// jacobi relax param
		int lap_damping;				// loops of laplacian damping
		float air_damping;				// damping of the air
		float bending_k;				// related to the thickness of the cloth
		float spring_k;					// related to the elasticity of the cloth
		float spring_k_raw;				// spring_k_raw / avgArea = spring_k
		float stitch_k;					// stiffness of stithed vertex, for sewing
		float stitch_k_raw;				// stitch_k_raw / avgArea = stitch_k
		float stitch_ratio;				// for each stitch, the length will -= ratio*time_step each update
		int out_iter;					// number of iterations
		int inner_iter;					// number of iterations
		float control_mag;				// for dragging, the stiffness of dragged point
		float time_step;				// simulation time step
		ldp::Float3 gravity;	
		SimulationParam();
		void setDefaultParam();
	};

	struct ClothDesignParam
	{
		float pointMergeDistThre;				// in meters
		float curveSampleStep;					// in meters; each curve is discreated via this step
		float pointInsidePolyThre;				// in meters

		ClothDesignParam();
		void setDefaultParam();
	};

	struct DragInfo
	{
		ObjMesh* selected_cloth;
		int selected_vert_id;
		ldp::Float3 target;
		DragInfo()
		{
			selected_cloth = nullptr;
			selected_vert_id = -1;
			target = 0;
		}
	};

	class ClothPiece;
	class PanelPolygon;
	class AbstractShape;
	class LevelSet3D;
	class BMesh;
	class BMVert;
	class ClothManager
	{
	public:
		enum
		{
			ClothIdxBegin = 1
		};
		typedef float ValueType;
		typedef ldp::ldp_basic_vec3<float> Vec3;
		typedef ldp::ldp_basic_vec2<float> Vec2;
		enum SimulationMode
		{
			SimulationNotInit,
			SimulationOn,
			SimulationPause,
		};
	protected:
		struct DragInfoInternal
		{
			int vert_id;
			ldp::Float3 dir;
			ldp::Float3 target;
			DragInfoInternal()
			{
				vert_id = -1;
			}
		};
	public:
		ClothManager();
		~ClothManager();

		void clear();

		// load cloth pieces from svg of my format.
		void loadPiecesFromSvg(std::string filename);

		/// simulation main functions
		void simulationInit();							// must be called after the body and all cloths ready.
		void simulationUpdate();
		void simulationDestroy();

		/// dragging related
		void dragBegin(DragInfo info);
		void dragMove(ldp::Float3 target);
		void dragEnd();

		/// parameters related
		void setSimulationMode(SimulationMode mode);
		void setSimulationParam(SimulationParam param);
		void setClothDesignParam(ClothDesignParam param);

		/// mesh backup related
		void updateCurrentClothsToInitial();
		void updateInitialClothsToCurrent();

		/// stitch related
		void clearstitches();
		void addStitchVert(const ClothPiece* cloth1, int mesh_vid1, const ClothPiece* cloth2, int mesh_vid2);

		/// getters
		float getFps()const { return m_fps; }
		SimulationMode getSimulationMode()const { return m_simulationMode; }
		SimulationParam getSimulationParam()const { return m_simulationParam; }
		ClothDesignParam getClothDesignParam()const { return m_clothDesignParam; }
		const ObjMesh* bodyMesh()const { return m_bodyMesh.get(); }
		ObjMesh* bodyMesh() { return m_bodyMesh.get(); }
		const LevelSet3D* bodyLevelSet()const { return m_bodyLvSet.get(); }
		LevelSet3D* bodyLevelSet() { return m_bodyLvSet.get(); }
		int numClothPieces()const { return (int)m_clothPieces.size(); }
		const ClothPiece* clothPiece(int i)const { return m_clothPieces.at(i).get(); }
		ClothPiece* clothPiece(int i) { return m_clothPieces.at(i).get(); }
		void clearClothPieces();
		void addClothPiece(std::shared_ptr<ClothPiece> piece);
		void removeClothPiece(int i);
		int numStitches()const { return (int)m_stitches.size(); }
		std::pair<Float3, Float3> getStitchPos(int i)const;
		void get2dBound(ldp::Float2& bmin, ldp::Float2& bmax)const;
	private:
		std::vector<std::shared_ptr<ClothPiece>> m_clothPieces;
		std::shared_ptr<ObjMesh> m_bodyMesh;
		std::shared_ptr<LevelSet3D> m_bodyLvSet;
		SimulationMode m_simulationMode;
		SimulationParam m_simulationParam;
		ClothDesignParam m_clothDesignParam;
		ValueType m_avgArea;
		ValueType m_avgEdgeLength;
		ValueType m_fps;
		bool m_shouldMergePieces;
		bool m_shouldTopologyUpdate;
		bool m_shouldNumericUpdate;
		bool m_shouldStitchUpdate;
		DragInfoInternal m_curDragInfo;
		// 2D-3D triangulation related---------------------------------------------------
	protected:
		bool pointInPolygon(int n, const Vec2* pts, Vec2 p);
		void triangulate();
		void polyPathToShape(const svg::SvgPolyPath* polyPath,
			std::vector<std::shared_ptr<AbstractShape>>& group, float pixel2meter);
		// Topology related--------------------------------------------------------------
	protected:
		void mergePieces();
		void buildTopology();
		void buildNumerical();
		void buildStitch();
		int findNeighbor(int i, int j)const;
		int findStitchNeighbor(int i, int j)const;
	private:
		std::shared_ptr<BMesh> m_bmesh;					// topology mesh
		std::vector<BMVert*> m_bmeshVerts;				// topology mesh
		std::map<const ObjMesh*, int> m_clothVertBegin;	// index begin of each cloth piece
		std::vector<Vec3> m_X;							// vertex position list
		std::vector<Vec3> m_V;							// vertex velocity list
		std::vector<Int3> m_T;							// triangle list
		std::vector<Int2> m_allE;						// edges + bending edges, sorted, for [0,1,2]+[0,1,3], bend_e=[2,3]
		std::vector<int> m_allVV;						// one-ring vertex of each vertex based an allE, NOT including self
		std::vector<ValueType> m_allVL;					// off-diag values of spring length
		std::vector<ValueType> m_allVW;					// off-diag values of springs
		std::vector<ValueType> m_allVC;					// diag values of springs
		std::vector<int> m_allVV_num;					// num of one-ring vertex of each vertex
		std::vector<ValueType> m_fixed;					// fix constraints of vertices
		std::vector<Int4> m_edgeWithBendEdge;			// original edges + beding edges, before sorted and unique.
		std::vector<Int2> m_stitches;					// the elements that must be stitched together, for sewing
		std::vector<int> m_stitchVV;
		std::vector<int> m_stitchVV_num;
		std::vector<ValueType> m_stitchVW;
		std::vector<ValueType> m_stitchVC;
		std::vector<ValueType> m_stitchVL;		
		ValueType m_curStitchRatio;
		// GPU related-------------------------------------------------------------------
	protected:
		void debug_save_values();

		///// kernel wrappers
		void laplaceDamping();						// apply laplacian damping
		void updateAfterLap();						// X += V(apply air damping, gravity, etc.
		void constrain0();							// compute init_B and new_VC
		void constrain1();							// inner loop, jacobi update
		void constrain2(float omega);				// inner loop, chebshev relax
		void constrain3();							// collision handle using level set.
		void constrain4();							// update velocity
		void resetMoreFixed();						// for draging
	private:
		DeviceArray<ValueType> m_dev_X;				// position
		DeviceArray<ValueType> m_dev_old_X;			// position backup
		DeviceArray<ValueType> m_dev_next_X;		// next X for temporary storage
		DeviceArray<ValueType> m_dev_prev_X;		// prev X for temporary storage
		DeviceArray<ValueType> m_dev_fixed;			// fixed constraint, indicating which vertex should be fixed
		DeviceArray<ValueType> m_dev_more_fixed;	// for dragging
		DeviceArray<ValueType> m_dev_V;				// velocity
		DeviceArray<ValueType> m_dev_init_B;		// Initialized momentum condition in B
		DeviceArray<int> m_dev_T;					// trangle list
		DeviceArray<int> m_dev_all_VV;				// one-ring vertex list, NOT including itself
		DeviceArray<int> m_dev_all_vv_num;			// csr index of allVV
		DeviceArray<ValueType> m_dev_all_VL;		// off-diagnal values
		DeviceArray<ValueType> m_dev_all_VW;		// off-diagnal values
		DeviceArray<ValueType> m_dev_all_VC;		// diagnal values
		DeviceArray<ValueType> m_dev_new_VC;		// diagnal values 
		DeviceArray<ValueType> m_dev_phi;			// level set values
		DeviceArray<int> m_dev_stitch_VV;
		DeviceArray<int> m_dev_stitch_VV_num;
		DeviceArray<ValueType> m_dev_stitch_VW;
		DeviceArray<ValueType> m_dev_stitch_VC;
		DeviceArray<ValueType> m_dev_stitch_VL;
#ifdef ENABLE_SELF_COLLISION
		COLLISION_HANDLER<ValueType> m_collider;
#endif
	};
}
