/*
 * Class_Para_Tree_3D.tpp
 *
 *  Created on: 23/apr/2014
 *      Author: Marco Cisternino
 */

// =================================================================================== //
// CLASS SPECIALIZATION                                                                //
// =================================================================================== //

template<>
class Class_Para_Tree<3>{
	// ------------------------------------------------------------------------------- //
	// TYPEDEFS ----------------------------------------------------------------------- //
public:
	typedef vector<Class_Octant<3> > 	OctantsType;
	typedef vector<uint32_t>			u32vector;
	typedef vector<double>				dvector;
	typedef vector<vector<uint32_t>	>	u32vector2D;
	typedef vector<vector<uint64_t>	>	u64vector2D;
	typedef vector<vector<double>	>	dvector2D;

	// ------------------------------------------------------------------------------- //
	// MEMBERS ----------------------------------------------------------------------- //
public:
	//undistributed members
	uint64_t* partition_first_desc; 			//global array containing position of the first possible octant in each processor
	uint64_t* partition_last_desc; 				//global array containing position of the last possible octant in each processor
	uint64_t* partition_range_globalidx;	 	//global array containing global index of the last existing octant in each processor
	uint64_t global_num_octants;   				// global number of octants in the parallel octree
	map<int,vector<uint32_t> > bordersPerProc;	//local indices of border octants per process
	int nproc;
	uint8_t max_depth;							// global max existing level in the parallel octree

	//distributed members
	int rank;
	Class_Local_Tree<3> octree;					// local tree in each processor

	//auxiliary members
	int error_flag;								// MPI error flag
	bool serial;								// 1 if the octree is the same on each processor, 0 if the octree is distributed

	//map member
	Class_Map<3> trans;

	// connectivity
	dvector2D					nodes;				// Local vector of nodes (x,y,z) ordered with Morton Number
	u32vector2D					connectivity;		// Local vector of connectivity (node1, node2, ...) ordered with Morton-order.
	// The nodes are stored as index of vector nodes
	dvector2D					ghostsnodes;		// Local vector of ghosts nodes (x,y,z) ordered with Morton Number
	u32vector2D					ghostsconnectivity;	// Local vector of ghosts connectivity (node1, node2, ...) ordered with Morton-order.
	// The nodes are stored as index of vector nodes

	// ------------------------------------------------------------------------------- //
	// CONSTRUCTORS ------------------------------------------------------------------ //
public:
	Class_Para_Tree(){
		serial = true;
		error_flag = 0;
		max_depth = 0;
		global_num_octants = octree.getNumOctants();
		error_flag = MPI_Comm_size(MPI_COMM_WORLD,&nproc);
		error_flag = MPI_Comm_rank(MPI_COMM_WORLD,&rank);
		partition_first_desc = new uint64_t[nproc];
		partition_last_desc = new uint64_t[nproc];
		partition_range_globalidx = new uint64_t[nproc];
		uint64_t lastDescMorton = octree.getLastDesc().computeMorton();
		uint64_t firstDescMorton = octree.getFirstDesc().computeMorton();
		for(int p = 0; p < nproc; ++p){
			partition_range_globalidx[p] = 0;
			partition_last_desc[p] = lastDescMorton;
			partition_last_desc[p] = firstDescMorton;
		}
		// Write info log
		if(rank==0){
			int sysError = system("rm PABLO.log");
		}
		MPI_Barrier(MPI_COMM_WORLD);
		writeLog("---------------------------------------------");
		writeLog("- PABLO PArallel Balanced Linear Octree -");
		writeLog("---------------------------------------------");
		writeLog(" ");
		writeLog("---------------------------------------------");
		writeLog(" Number of proc		:	" + to_string(nproc));
		writeLog(" Dimension		:	" + to_string(3));
		writeLog(" Max allowed level	:	" + to_string(MAX_LEVEL_3D));
		writeLog("---------------------------------------------");
		writeLog(" ");

	};

	//=================================================================================//

	Class_Para_Tree(double & X, double & Y, double & Z, double & L){
		serial = true;
		error_flag = 0;
		max_depth = 0;
		global_num_octants = octree.getNumOctants();
		error_flag = MPI_Comm_size(MPI_COMM_WORLD,&nproc);
		error_flag = MPI_Comm_rank(MPI_COMM_WORLD,&rank);
		partition_first_desc = new uint64_t[nproc];
		partition_last_desc = new uint64_t[nproc];
		partition_range_globalidx = new uint64_t[nproc];
		uint64_t lastDescMorton = octree.getLastDesc().computeMorton();
		uint64_t firstDescMorton = octree.getFirstDesc().computeMorton();
		for(int p = 0; p < nproc; ++p){
			partition_range_globalidx[p] = 0;
			partition_last_desc[p] = lastDescMorton;
			partition_last_desc[p] = firstDescMorton;
		}
		// Write info log
		if(rank==0){
			int sysError = system("rm PABLO.log");
		}
		MPI_Barrier(MPI_COMM_WORLD);
		writeLog("---------------------------------------------");
		writeLog("- PABLO PArallel Balanced Linear Octree -");
		writeLog("---------------------------------------------");
		writeLog(" ");
		writeLog("---------------------------------------------");
		writeLog(" Number of proc		:	" + to_string(nproc));
		writeLog(" Dimension		:	" + to_string(3));
		writeLog(" Max allowed level	:	" + to_string(MAX_LEVEL_3D));
		writeLog(" Domain Origin		:	" + to_string(X));
		writeLog("				" + to_string(Y));
		writeLog("				" + to_string(Z));
		writeLog(" Domain Size		:	" + to_string(L));
		writeLog("---------------------------------------------");
		writeLog(" ");

	};

	//=================================================================================//

	~Class_Para_Tree(){
		writeLog("---------------------------------------------");
		writeLog("--------------- R.I.P. PABLO ----------------");
		writeLog("---------------------------------------------");
		writeLog("---------------------------------------------");
	};

	// =============================================================================== //
	// GET/SET METHODS ----------------------------------------------------------------------- //

public:
	double getX(Class_Octant<3>* const oct){
		return trans.mapX(oct->getX());
	};

	double getY(Class_Octant<3>* const oct){
		return trans.mapY(oct->getY());
	};

	double getZ(Class_Octant<3>* const oct){
		return trans.mapZ(oct->getZ());
	};

	double getSize(Class_Octant<3>* const oct){		// Get the size of octant if mapped in hypercube
		return trans.mapSize(oct->getSize());
	};

	double getArea(Class_Octant<3>* const oct){		// Get the face area of octant
		return trans.mapArea(oct->getArea());
	};

	double getVolume(Class_Octant<3>* const oct){		// Get the volume of octant
		return trans.mapVolume(oct->getVolume());
	};

	void getCenter(Class_Octant<3>* oct, 			// Get a vector of DIM with the coordinates of the center of octant
				dvector & center){
		double* center_ = oct->getCenter();
		trans.mapCenter(center_, center);
		delete [] center_;
		center_ = NULL;
	};

	void getNodes(Class_Octant<3>* oct, 			// Get a vector of vector (size [nnodes][DIM]) with the nodes of octant
				dvector2D & nodes){
		uint32_t (*nodes_)[3] = oct->getNodes();
		trans.mapNodes(nodes_, nodes);
		delete [] nodes_;
		nodes_ = NULL;
	};

	void getNormal(Class_Octant<3>* oct, 			// Get a vector of vector (size [DIM]) with the normal of the iface
					uint8_t & iface,
					dvector & normal){
		vector<int8_t> normal_;
		oct->getNormal(iface, normal_);
		trans.mapNormals(normal_, normal);

	};

	// =============================================================================== //

	Class_Octant<3>* getPointOwner(dvector & point){			// Get the pointer to the octant owner of an input point
																// (vector<double> with x,y,z). If the point is out of process
																// return NULL.
		uint32_t noctants = octree.octants.size();
		uint32_t idxtry = noctants/2;
		uint32_t x, y, z;
		uint64_t morton, mortontry;
		int powner;

		x = trans.mapX(point[0]);
		y = trans.mapX(point[1]);
		z = trans.mapX(point[2]);
		morton = mortonEncode_magicbits(x,y,z);
		powner = findOwner(morton);
		if (powner!=rank)
			return NULL;

		int32_t jump = idxtry;
		while(abs(jump) > 0){
			mortontry = octree.octants[idxtry].computeMorton();
			jump = ((mortontry<morton)-(mortontry>morton))*abs(jump)/2;
			idxtry += jump;
			if (idxtry > noctants-1){
				if (jump > 0){
					idxtry = noctants - 1;
					jump = 0;
				}
				else if (jump < 0){
					idxtry = 0;
					jump = 0;
				}
			}
		}
		if(octree.octants[idxtry].computeMorton() == morton){
			return &octree.octants[idxtry];
		}
		else{
			// Step until the mortontry lower than morton (one idx of distance)
			{
				while(octree.octants[idxtry].computeMorton() < morton){
					idxtry++;
					if(idxtry > noctants-1){
						idxtry = noctants-1;
						break;
					}
				}
				while(octree.octants[idxtry].computeMorton() > morton){
					idxtry--;
					if(idxtry > noctants-1){
						idxtry = noctants-1;
						break;
					}
				}
			}
			return &octree.octants[idxtry];
		}

	};

	// =============================================================================== //

	double getSize(Class_Intersection<3>* const inter) {
		uint32_t Size;
		Size = octree.extractOctant(inter->owners[inter->finer]).getSize();
		return trans.mapSize(Size);
	}

	double getArea(Class_Intersection<3>* const inter) {
		uint32_t Area;
		Area = octree.extractOctant(inter->owners[inter->finer]).getArea();
		return trans.mapArea(Area);
	}

	void getCenter(Class_Intersection<3>* inter,
			vector<double>& center) {
		Class_Octant<3> oct = octree.extractOctant(inter->owners[inter->finer]);
		double* center_ = oct.getCenter();
		trans.mapCenter(center_, center);
		delete [] center_;
		center_ = NULL;
	}

	void getNodes(Class_Intersection<3>* const inter,
			dvector2D & nodes) {
		Class_Octant<3> oct = octree.extractOctant(inter->owners[inter->finer]);
		uint8_t iface = inter->iface;
		uint32_t (*nodes_all)[3] = oct.getNodes();
		uint32_t (*nodes_)[3] = new uint32_t[global3D.nnodesperface][3];
		for (int i=0; i<global2D.nnodesperface; i++){
			for (int j=0; j<3; j++){
				nodes_[i][j] = nodes_all[global3D.facenode[iface][i]][j];
			}
		}
		trans.mapNodesIntersection(nodes_, nodes);
		delete [] nodes_;
		nodes_ = NULL;
	}

	void getNormal(Class_Intersection<3>* const inter,
			dvector & normal) {
		Class_Octant<3> oct = octree.extractOctant(inter->owners[inter->finer]);
		uint8_t iface = inter->iface;
		vector<int8_t> normal_;
		oct.getNormal(iface, normal_);
		trans.mapNormals(normal_, normal);
	}

	// =============================================================================== //
	// METHODS ----------------------------------------------------------------------- //

	void computePartition(uint32_t* partition){ 		// compute octant partition giving the same number of octant to each process and redistributing the reminder
		uint32_t division_result = 0;
		uint32_t remind = 0;
		division_result = uint32_t(global_num_octants/(uint64_t)nproc);
		remind = (uint32_t)(global_num_octants%(uint64_t)nproc);
		for(int i = 0; i < nproc; ++i)
			if(i<remind)
				partition[i] = division_result + 1;
			else
				partition[i] = division_result;
	};

	//=================================================================================//

	void computePartition(uint32_t* partition,	 		// compute octant partition giving almost the same number of octant to each process
			uint8_t & level_){   						// with complete families contained in octants of n "level" over the leaf in each process
		//	level = uint8_t(min(int(level), MAX_LEVEL));
		uint8_t level = uint8_t(min(int(max(int(max_depth) - int(level_), int(1))) , MAX_LEVEL_3D));
		uint32_t partition_temp[nproc];
		uint8_t boundary_proc[nproc-1], dimcomm, glbdimcomm[nproc], indcomm, glbindcomm[nproc];
		uint32_t division_result = 0;
		uint32_t remind = 0;
		uint32_t Dh = uint32_t(pow(double(2),double(MAX_LEVEL_3D-level)));
		uint32_t istart, nocts, rest, forw, backw;
		uint32_t i = 0, iproc, j;
		uint64_t sum;
		int32_t deplace[nproc-1], *pointercomm;
		division_result = uint32_t(global_num_octants/(uint64_t)nproc);
		remind = (uint32_t)(global_num_octants%(uint64_t)nproc);
		for(int i = 0; i < nproc; ++i)
			if(i<remind)
				partition_temp[i] = division_result + 1;
			else
				partition_temp[i] = division_result;

		j = 0;
		sum = 0;
		for (iproc=0; iproc<nproc-1; iproc++){
			sum += partition_temp[iproc];
			while(sum > partition_range_globalidx[j]){
				j++;
			}
			boundary_proc[iproc] = j;
		}
		nocts = octree.octants.size();
		sum = 0;
		dimcomm = 0;
		indcomm = 0;
		for (iproc=0; iproc<nproc-1; iproc++){
			deplace[iproc] = 1;
			sum += partition_temp[iproc];
			if (boundary_proc[iproc] == rank){
				if (dimcomm == 0){
					indcomm = iproc;
				}
				dimcomm++;
				if (rank!=0)
					istart = sum - partition_range_globalidx[rank-1] - 1;
				else
					istart = sum;

				i = istart;
				rest = octree.octants[i].getX()%Dh + octree.octants[i].getY()%Dh + octree.octants[i].getZ()%Dh;
				while(rest!=0){
					if (i==nocts){
						i = istart + nocts;
						break;
					}
					i++;
					rest = octree.octants[i].getX()%Dh + octree.octants[i].getY()%Dh + octree.octants[i].getZ()%Dh;
				}
				forw = i - istart;
				i = istart;
				rest = octree.octants[i].getX()%Dh + octree.octants[i].getY()%Dh + octree.octants[i].getZ()%Dh;
				while(rest!=0){
					if (i==0){
						i = istart - nocts;
						break;
					}
					i--;
					rest = octree.octants[i].getX()%Dh + octree.octants[i].getY()%Dh + octree.octants[i].getZ()%Dh;
				}
				backw = istart - i;
				if (forw<backw)
					deplace[iproc] = forw;
				else
					deplace[iproc] = -backw;
			}
		}

		error_flag = MPI_Allgather(&dimcomm,1,MPI_UINT8_T,glbdimcomm,1,MPI_UINT8_T,MPI_COMM_WORLD);
		error_flag = MPI_Allgather(&indcomm,1,MPI_UINT8_T,glbindcomm,1,MPI_UINT8_T,MPI_COMM_WORLD);
		for (iproc=0; iproc<nproc; iproc++){
			pointercomm = &deplace[glbindcomm[iproc]];
			error_flag = MPI_Bcast(pointercomm, glbdimcomm[iproc], MPI_INT32_T, iproc, MPI_COMM_WORLD);
		}

		for (iproc=0; iproc<nproc; iproc++){
			if (iproc < nproc-1)
				partition[iproc] = partition_temp[iproc] + deplace[iproc];
			else
				partition[iproc] = partition_temp[iproc];
			if (iproc !=0)
				partition[iproc] = partition[iproc] - deplace[iproc-1];
		}
	};

	//=================================================================================//

	void updateLoadBalance(){							//update Class_Para_Tree members after a load balance
		octree.updateLocalMaxDepth();
		//update partition_range_globalidx
		uint64_t rbuff [nproc];
		uint64_t local_num_octants = octree.getNumOctants();
		error_flag = MPI_Allgather(&local_num_octants,1,MPI_UINT64_T,&rbuff,1,MPI_UINT64_T,MPI_COMM_WORLD);
		for(int p = 0; p < nproc; ++p){
			partition_range_globalidx[p] = 0;
			for(int pp = 0; pp <=p; ++pp)
				partition_range_globalidx[p] += rbuff[pp];
			--partition_range_globalidx[p];
		}
		//update first last descendant
		octree.setFirstDesc();
		octree.setLastDesc();
		//update partition_range_position
		uint64_t lastDescMorton = octree.getLastDesc().computeMorton();
		error_flag = MPI_Allgather(&lastDescMorton,1,MPI_UINT64_T,partition_last_desc,1,MPI_UINT64_T,MPI_COMM_WORLD);
		uint64_t firstDescMorton = octree.getFirstDesc().computeMorton();
		error_flag = MPI_Allgather(&firstDescMorton,1,MPI_UINT64_T,partition_first_desc,1,MPI_UINT64_T,MPI_COMM_WORLD);
		serial = false;
	};

	//=================================================================================//

	int findOwner(const uint64_t & morton){				// given the morton of an octant it finds the process owning that octant
		int p = -1;
		int length = nproc;
		int beg = 0;
		int end = nproc -1;
		int seed = nproc/2;
		while(beg != end){
			if(morton <= partition_last_desc[seed]){
				end = seed;
				//			length = seed + 1;
				if(morton > partition_last_desc[seed-1])
					beg = seed;
			}
			else{
				beg = seed;
				if(morton <= partition_last_desc[seed+1])
					beg = seed + 1;
				//	length = end - seed -1;
			}
			length = end - beg;
			seed = beg + length/2;
		}
		p = beg;
		return p;
	};

	//=================================================================================//

	void setPboundGhosts(){
		//BUILD BORDER OCTANT INDECES VECTOR (map value) TO BE SENT TO THE RIGHT PROCESS (map key)
		//find local octants to be sent as ghost to the right processes
		//it visits the local octants building virtual neighbors on each octant face
		//find the owner of these virtual neighbor and build a map (process,border octants)
		//this map contains the local octants as ghosts for neighbor processes
		if(octree.pborders.size() == 0){
			Class_Local_Tree<3>::OctantsType::iterator end = octree.octants.end();
			Class_Local_Tree<3>::OctantsType::iterator begin = octree.octants.begin();
			bordersPerProc.clear();
			for(Class_Local_Tree<3>::OctantsType::iterator it = begin; it != end; ++it){
				set<int> procs;
				//Virtual Face Neighbors
				for(uint8_t i = 0; i < global3D.nfaces; ++i){
					if(it->getBound(i) == false){
						uint32_t virtualNeighborsSize = 0;
						uint64_t* virtualNeighbors = it->computeVirtualMorton(i,max_depth,virtualNeighborsSize);
						uint32_t maxDelta = virtualNeighborsSize/2;
						for(int j = 0; j <= maxDelta; ++j){
							int pBegin = findOwner(virtualNeighbors[j]);
							int pEnd = findOwner(virtualNeighbors[virtualNeighborsSize - 1 - j]);
							procs.insert(pBegin);
							procs.insert(pEnd);
							if(pBegin != rank || pEnd != rank){
								it->setPbound(i,true);
							}
							else{
								it->setPbound(i,false);
							}
							//						if(pEnd == rank && pEnd == rank)
							//							break;
						}
						delete [] virtualNeighbors;
						virtualNeighbors = NULL;
					}
				}
				//Virtual Edge Neighbors
				for(uint8_t e = 0; e < global3D.nedges; ++e){
					uint32_t virtualEdgeNeighborSize = 0;
					uint64_t* virtualEdgeNeighbors = it->computeEdgeVirtualMorton(e,max_depth,virtualEdgeNeighborSize);
					uint32_t maxDelta = virtualEdgeNeighborSize/2;
					if(virtualEdgeNeighborSize){
						for(int ee = 0; ee <= maxDelta; ++ee){
							int pBegin = findOwner(virtualEdgeNeighbors[ee]);
							int pEnd = findOwner(virtualEdgeNeighbors[virtualEdgeNeighborSize - 1- ee]);
							procs.insert(pBegin);
							procs.insert(pEnd);
							//						if(pEnd == rank && pEnd == rank)
							//							break;
						}
					}
					delete [] virtualEdgeNeighbors;
					virtualEdgeNeighbors = NULL;
				}
				//Virtual Corner Neighbors
				for(uint8_t c = 0; c < global3D.nnodes; ++c){
					uint32_t virtualCornerNeighborSize = 0;
					uint64_t virtualCornerNeighbor = it ->computeNodeVirtualMorton(c,max_depth,virtualCornerNeighborSize);
					if(virtualCornerNeighborSize){
						int proc = findOwner(virtualCornerNeighbor);
						procs.insert(proc);
					}
				}

				set<int>::iterator pitend = procs.end();
				for(set<int>::iterator pit = procs.begin(); pit != pitend; ++pit){
					int p = *pit;
					if(p != rank){
						//TODO better reserve to avoid if
						bordersPerProc[p].push_back(distance(begin,it));
						vector<uint32_t> & bordersSingleProc = bordersPerProc[p];
						if(bordersSingleProc.capacity() - bordersSingleProc.size() < 2)
							bordersSingleProc.reserve(2*bordersSingleProc.size());
					}
				}
			}
		}
		else{
			Class_Local_Tree<3>::u32vector::iterator end = octree.pborders.end();
			Class_Local_Tree<3>::u32vector::iterator begin = octree.pborders.begin();
			bordersPerProc.clear();
			for(Class_Local_Tree<3>::u32vector::iterator it = begin; it != end; ++it){
				Class_Octant<3> & oct = octree.octants[*it];
				set<int> procs;
				//Virtual Face Neighbors
				for(uint8_t i = 0; i < global3D.nfaces; ++i){
					if(oct.getBound(i) == false){
						uint32_t virtualNeighborsSize = 0;
						uint64_t* virtualNeighbors = oct.computeVirtualMorton(i,max_depth,virtualNeighborsSize);
						uint32_t maxDelta = virtualNeighborsSize/2;
						for(int j = 0; j <= maxDelta; ++j){
							int pBegin = findOwner(virtualNeighbors[j]);
							int pEnd = findOwner(virtualNeighbors[virtualNeighborsSize - 1 - j]);
							procs.insert(pBegin);
							procs.insert(pEnd);
							if(pBegin == pEnd || pBegin == pEnd - 1)
								break;
						}
						delete [] virtualNeighbors;
						virtualNeighbors = NULL;
					}
				}
				//Virtual Edge Neighbors
				for(uint8_t e = 0; e < global3D.nedges; ++e){
					uint32_t virtualEdgeNeighborSize = 0;
					uint64_t* virtualEdgeNeighbors = oct.computeEdgeVirtualMorton(e,max_depth,virtualEdgeNeighborSize);
					uint32_t maxDelta = virtualEdgeNeighborSize/2;
					if(virtualEdgeNeighborSize){
						for(int ee = 0; ee <= maxDelta; ++ee){
							int pBegin = findOwner(virtualEdgeNeighbors[ee]);
							int pEnd = findOwner(virtualEdgeNeighbors[virtualEdgeNeighborSize - 1- ee]);
							procs.insert(pBegin);
							procs.insert(pEnd);
							//						if(pEnd == rank && pEnd == rank)
							//							break;
						}
					}
					delete [] virtualEdgeNeighbors;
					virtualEdgeNeighbors = NULL;
				}
				//Virtual Corner Neighbors
				for(uint8_t c = 0; c < global3D.nnodes; ++c){
					uint32_t virtualCornerNeighborSize = 0;
					uint64_t virtualCornerNeighbor = oct.computeNodeVirtualMorton(c,max_depth,virtualCornerNeighborSize);
					if(virtualCornerNeighborSize){
						int proc = findOwner(virtualCornerNeighbor);
						procs.insert(proc);
					}
				}

				set<int>::iterator pitend = procs.end();
				for(set<int>::iterator pit = procs.begin(); pit != pitend; ++pit){
					int p = *pit;
					if(p != rank){
						//TODO better reserve to avoid if
						bordersPerProc[p].push_back(*it);
						vector<uint32_t> & bordersSingleProc = bordersPerProc[p];
						if(bordersSingleProc.capacity() - bordersSingleProc.size() < 2)
							bordersSingleProc.reserve(2*bordersSingleProc.size());
					}
				}
			}

		}
		MPI_Barrier(MPI_COMM_WORLD);

		//PACK (mpi) BORDER OCTANTS IN CHAR BUFFERS WITH SIZE (map value) TO BE SENT TO THE RIGHT PROCESS (map key)
		//it visits every element in bordersPerProc (one for every neighbor proc)
		//for every element it visits the border octants it contains and pack them in a new structure, sendBuffers
		//this map has an entry Class_Comm_Buffer for every proc containing the size in bytes of the buffer and the octants
		//to be sent to that proc packed in a char* buffer
		uint32_t x,y,z;
		uint8_t l;
		int8_t m;
		bool info[16];
		map<int,Class_Comm_Buffer> sendBuffers;
		map<int,vector<uint32_t> >::iterator bitend = bordersPerProc.end();
		uint32_t pbordersOversize = 0;
		for(map<int,vector<uint32_t> >::iterator bit = bordersPerProc.begin(); bit != bitend; ++bit){
			pbordersOversize += bit->second.size();
			int buffSize = bit->second.size() * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));// + (int)ceil((double)sizeof(int)/(double)(CHAR_BIT/8));
			int key = bit->first;
			const vector<uint32_t> & value = bit->second;
			sendBuffers[key] = Class_Comm_Buffer(buffSize,'a');
			int pos = 0;
			int nofBorders = value.size();
			for(int i = 0; i < nofBorders; ++i){
				//the use of auxiliary variable can be avoided passing to MPI_Pack the members of octant but octant in that case cannot be const
				const Class_Octant<3> & octant = octree.octants[value[i]];
				x = octant.getX();
				y = octant.getY();
				z = octant.getZ();
				l = octant.getLevel();
				m = octant.getMarker();
				memcpy(info,octant.info,16);
				error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[key].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
				error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[key].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
				error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[key].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
				error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[key].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
				error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[key].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
				for(int j = 0; j < 16; ++j){
					MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[key].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
				}
			}
		}

		//Build pborders
		octree.pborders.clear();
		//	octree.pborders.reserve(pbordersOversize);
		//	for(map<int,vector<uint32_t> >::iterator bit = bordersPerProc.begin(); bit != bitend; ++bit){
		//		set_union(bit->second.begin(),bit->second.end(),octree.pborders.begin(),octree.pborders.end(),octree.pborders.begin());
		//	}

		//COMMUNICATE THE SIZE OF BUFFER TO THE RECEIVERS
		//the size of every borders buffer is communicated to the right process in order to build the receive buffer
		//and stored in the recvBufferSizePerProc structure
		MPI_Request req[sendBuffers.size()*2];
		MPI_Status stats[sendBuffers.size()*2];
		int nReq = 0;
		map<int,int> recvBufferSizePerProc;
		map<int,Class_Comm_Buffer>::iterator sitend = sendBuffers.end();
		for(map<int,Class_Comm_Buffer>::iterator sit = sendBuffers.begin(); sit != sitend; ++sit){
			recvBufferSizePerProc[sit->first] = 0;
			error_flag = MPI_Irecv(&recvBufferSizePerProc[sit->first],1,MPI_UINT32_T,sit->first,rank,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		map<int,Class_Comm_Buffer>::reverse_iterator rsitend = sendBuffers.rend();
		for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
			error_flag =  MPI_Isend(&rsit->second.commBufferSize,1,MPI_UINT32_T,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		MPI_Waitall(nReq,req,stats);

		//COMMUNICATE THE BUFFERS TO THE RECEIVERS
		//recvBuffers structure is declared and each buffer is initialized to the right size
		//then, sendBuffers are communicated by senders and stored in recvBuffers in the receivers
		//at the same time every process compute the size in bytes of all the ghost octants
		uint32_t nofBytesOverProc = 0;
		map<int,Class_Comm_Buffer> recvBuffers;
		map<int,int>::iterator ritend = recvBufferSizePerProc.end();
		for(map<int,int>::iterator rit = recvBufferSizePerProc.begin(); rit != ritend; ++rit){
			recvBuffers[rit->first] = Class_Comm_Buffer(rit->second,'a');
		}
		nReq = 0;
		for(map<int,Class_Comm_Buffer>::iterator sit = sendBuffers.begin(); sit != sitend; ++sit){
			nofBytesOverProc += recvBuffers[sit->first].commBufferSize;
			error_flag = MPI_Irecv(recvBuffers[sit->first].commBuffer,recvBuffers[sit->first].commBufferSize,MPI_PACKED,sit->first,rank,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
			error_flag =  MPI_Isend(rsit->second.commBuffer,rsit->second.commBufferSize,MPI_PACKED,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		MPI_Waitall(nReq,req,stats);

		//COMPUTE GHOSTS SIZE IN BYTES
		//number of ghosts in every process is obtained through the size in bytes of the single octant
		//and ghost vector in local tree is resized
		uint32_t nofGhosts = nofBytesOverProc / (uint32_t)global3D.octantBytes;
		octree.size_ghosts = nofGhosts;
		octree.ghosts.resize(nofGhosts);

		//UNPACK BUFFERS AND BUILD GHOSTS CONTAINER OF CLASS_LOCAL_TREE
		//every entry in recvBuffers is visited, each buffers from neighbor processes is unpacked octant by octant.
		//every ghost octant is built and put in the ghost vector
		uint32_t ghostCounter = 0;
		map<int,Class_Comm_Buffer>::iterator rritend = recvBuffers.end();
		for(map<int,Class_Comm_Buffer>::iterator rrit = recvBuffers.begin(); rrit != rritend; ++rrit){
			int pos = 0;
			int nofGhostsPerProc = int(rrit->second.commBufferSize / (uint32_t) global3D.octantBytes);
			for(int i = 0; i < nofGhostsPerProc; ++i){
				error_flag = MPI_Unpack(rrit->second.commBuffer,rrit->second.commBufferSize,&pos,&x,1,MPI_UINT32_T,MPI_COMM_WORLD);
				error_flag = MPI_Unpack(rrit->second.commBuffer,rrit->second.commBufferSize,&pos,&y,1,MPI_UINT32_T,MPI_COMM_WORLD);
				error_flag = MPI_Unpack(rrit->second.commBuffer,rrit->second.commBufferSize,&pos,&z,1,MPI_UINT32_T,MPI_COMM_WORLD);
				error_flag = MPI_Unpack(rrit->second.commBuffer,rrit->second.commBufferSize,&pos,&l,1,MPI_UINT8_T,MPI_COMM_WORLD);
				octree.ghosts[ghostCounter] = Class_Octant<3>(l,x,y,z);
				error_flag = MPI_Unpack(rrit->second.commBuffer,rrit->second.commBufferSize,&pos,&m,1,MPI_INT8_T,MPI_COMM_WORLD);
				octree.ghosts[ghostCounter].setMarker(m);
				for(int j = 0; j < 16; ++j){
					error_flag = MPI_Unpack(rrit->second.commBuffer,rrit->second.commBufferSize,&pos,&info[j],1,MPI::BOOL,MPI_COMM_WORLD);
					octree.ghosts[ghostCounter].info[j] = info[j];
				}
				//			octree.ghosts[ghostCounter].info[15] = true;
				++ghostCounter;
			}
		}
		recvBuffers.clear();
		sendBuffers.clear();
		recvBufferSizePerProc.clear();

	}; 			 		// set pbound and build ghosts after static load balance

	//=================================================================================//

	void loadBalance(){									//assign the octants to the processes following a computed partition

		//Write info on log
		writeLog("---------------------------------------------");
		writeLog(" LOAD BALANCE ");

		uint32_t* partition = new uint32_t [nproc];
		computePartition(partition);
		if(serial)
		{
			writeLog(" ");
			writeLog(" Initial Serial distribution : ");
			for(int ii=0; ii<nproc; ii++){
				writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]+1));
			}

			uint32_t stride = 0;
			for(int i = 0; i < rank; ++i)
				stride += partition[i];
			Class_Local_Tree<3>::OctantsType::const_iterator first = octree.octants.begin() + stride;
			Class_Local_Tree<3>::OctantsType::const_iterator last = first + partition[rank];
			octree.octants.assign(first, last);
			octree.octants.shrink_to_fit();
			first = octree.octants.end();
			last = octree.octants.end();

			//Update and ghosts here
			updateLoadBalance();
			setPboundGhosts();

		}
		else
		{
			writeLog(" ");
			writeLog(" Initial Parallel partition : ");
			writeLog(" Octants for proc	"+ to_string(0)+"	:	" + to_string(partition_range_globalidx[0]+1));
			for(int ii=1; ii<nproc; ii++){
				writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]-partition_range_globalidx[ii-1]));
			}

			//empty ghosts
			octree.ghosts.clear();
			octree.size_ghosts = 0;
			//compute new partition range globalidx
			uint64_t* newPartitionRangeGlobalidx = new uint64_t[nproc];
			for(int p = 0; p < nproc; ++p){
				newPartitionRangeGlobalidx[p] = 0;
				for(int pp = 0; pp <= p; ++pp)
					newPartitionRangeGlobalidx[p] += (uint64_t)partition[pp];
				--newPartitionRangeGlobalidx[p];
			}

			//find resident octants local offset lastHead(lh) and firstTail(ft)
			int32_t lh,ft;
			if(rank == 0)
				lh = -1;
			else{
				lh = (int32_t)(newPartitionRangeGlobalidx[rank-1] + 1 - partition_range_globalidx[rank-1] - 1 - 1);
			}
			if(lh < 0)
				lh = - 1;
			else if(lh > octree.octants.size() - 1)
				lh = octree.octants.size() - 1;

			if(rank == nproc - 1)
				ft = octree.octants.size();
			else if(rank == 0)
				ft = (int32_t)(newPartitionRangeGlobalidx[rank] + 1);
			else{
				ft = (int32_t)(newPartitionRangeGlobalidx[rank] - partition_range_globalidx[rank -1]);
			}
			if(ft > (int32_t)(octree.octants.size() - 1))
				ft = octree.octants.size();
			else if(ft < 0)
				ft = 0;

			//compute size Head and size Tail
			uint32_t headSize = (uint32_t)(lh + 1);
			uint32_t tailSize = (uint32_t)(octree.octants.size() - ft);
			uint32_t headOffset = headSize;
			uint32_t tailOffset = tailSize;

			//build send buffers
			map<int,Class_Comm_Buffer> sendBuffers;

			//Compute first predecessor and first successor to send buffers to
			int64_t firstOctantGlobalIdx = 0;// offset to compute global index of each octant in every process
			int64_t globalLastHead = (int64_t) lh;
			int64_t globalFirstTail = (int64_t) ft; //lastHead and firstTail in global ordering
			int firstPredecessor = -1;
			int firstSuccessor = nproc;
			if(rank != 0){
				firstOctantGlobalIdx = (int64_t)(partition_range_globalidx[rank-1] + 1);
				globalLastHead = firstOctantGlobalIdx + (int64_t)lh;
				globalFirstTail = firstOctantGlobalIdx + (int64_t)ft;
				for(int pre = rank - 1; pre >=0; --pre){
					if((uint64_t)globalLastHead <= newPartitionRangeGlobalidx[pre])
						firstPredecessor = pre;
				}
				for(int post = rank + 1; post < nproc; ++post){
					if((uint64_t)globalFirstTail <= newPartitionRangeGlobalidx[post] && (uint64_t)globalFirstTail > newPartitionRangeGlobalidx[post-1])
						firstSuccessor = post;
				}
			}
			else if(rank == 0){
				firstSuccessor = 1;
			}
			MPI_Barrier(MPI_COMM_WORLD); //da spostare prima della prima comunicazione

			uint32_t x,y,z;
			uint8_t l;
			int8_t m;
			bool info[16];
			//build send buffers from Head
			if(headSize != 0){
				for(int p = firstPredecessor; p >= 0; --p){
					if(headSize <=partition[p]){
						int buffSize = headSize * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						int pos = 0;
						for(uint32_t i = 0; i <= lh; ++i){
							//PACK octants from 0 to lh in sendBuffer[p]
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							}
						}
						break;
					}
					else{
						int buffSize = partition[p] * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						int pos = 0;
						for(uint32_t i = lh - partition[p] + 1; i <= lh; ++i){
							//pack octants from lh - partition[p] to lh
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							}
						}
						lh -= partition[p];
						headSize = lh + 1;
					}
				}

			}
			//build send buffers from Tail
			if(tailSize != 0){
				for(int p = firstSuccessor; p < nproc; ++p){
					if(tailSize <= partition[p]){
						int buffSize = tailSize * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						int pos = 0;
						uint32_t octantsSize = (uint32_t)octree.octants.size();
						for(uint32_t i = ft; i < octantsSize; ++i){
							//PACK octants from ft to octantsSize-1
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							}
						}
						break;
					}
					else{
						int buffSize = partition[p] * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						uint32_t endOctants = ft + partition[p] - 1;
						int pos = 0;
						for(uint32_t i = ft; i <= endOctants; ++i ){
							//PACK octants from ft to ft + partition[p] -1
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							}
						}
						ft += partition[p];
						tailSize -= partition[p];
					}
				}
			}

			//Build receiver sources
			vector<Class_Array> recvs(nproc);
			recvs[rank] = Class_Array((uint32_t)sendBuffers.size()+1,-1);
			recvs[rank].array[0] = rank;
			int counter = 1;
			map<int,Class_Comm_Buffer>::iterator sitend = sendBuffers.end();
			for(map<int,Class_Comm_Buffer>::iterator sit = sendBuffers.begin(); sit != sitend; ++sit){
				recvs[rank].array[counter] = sit->first;
				++counter;
			}
			int nofRecvsPerProc[nproc];
			error_flag = MPI_Allgather(&recvs[rank].arraySize,1,MPI_INT,nofRecvsPerProc,1,MPI_INT,MPI_COMM_WORLD);
			int globalRecvsBuffSize = 0;
			int displays[nproc];
			for(int pp = 0; pp < nproc; ++pp){
				displays[pp] = 0;
				globalRecvsBuffSize += nofRecvsPerProc[pp];
				for(int ppp = 0; ppp < pp; ++ppp){
					displays[pp] += nofRecvsPerProc[ppp];
				}
			}
			int globalRecvsBuff[globalRecvsBuffSize];
			error_flag = MPI_Allgatherv(recvs[rank].array,recvs[rank].arraySize,MPI_INT,globalRecvsBuff,nofRecvsPerProc,displays,MPI_INT,MPI_COMM_WORLD);

			vector<set<int> > sendersPerProc(nproc);
			for(int pin = 0; pin < nproc; ++pin){
				for(int k = displays[pin]+1; k < displays[pin] + nofRecvsPerProc[pin]; ++k){
					sendersPerProc[globalRecvsBuff[k]].insert(globalRecvsBuff[displays[pin]]);
				}
			}

			//Communicate Octants (size)
			MPI_Request req[sendBuffers.size()+sendersPerProc[rank].size()];
			MPI_Status stats[sendBuffers.size()+sendersPerProc[rank].size()];
			int nReq = 0;
			map<int,int> recvBufferSizePerProc;
			set<int>::iterator senditend = sendersPerProc[rank].end();
			for(set<int>::iterator sendit = sendersPerProc[rank].begin(); sendit != senditend; ++sendit){
				recvBufferSizePerProc[*sendit] = 0;
				error_flag = MPI_Irecv(&recvBufferSizePerProc[*sendit],1,MPI_UINT32_T,*sendit,rank,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			map<int,Class_Comm_Buffer>::reverse_iterator rsitend = sendBuffers.rend();
			for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
				error_flag =  MPI_Isend(&rsit->second.commBufferSize,1,MPI_UINT32_T,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			MPI_Waitall(nReq,req,stats);

			//COMMUNICATE THE BUFFERS TO THE RECEIVERS
			//recvBuffers structure is declared and each buffer is initialized to the right size
			//then, sendBuffers are communicated by senders and stored in recvBuffers in the receivers
			uint32_t nofNewHead = 0;
			uint32_t nofNewTail = 0;
			map<int,Class_Comm_Buffer> recvBuffers;
			map<int,int>::iterator ritend = recvBufferSizePerProc.end();
			for(map<int,int>::iterator rit = recvBufferSizePerProc.begin(); rit != ritend; ++rit){
				recvBuffers[rit->first] = Class_Comm_Buffer(rit->second,'a');
				uint32_t nofNewPerProc = (uint32_t)(rit->second / (uint32_t)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8)));
				if(rit->first < rank)
					nofNewHead += nofNewPerProc;
				else if(rit->first > rank)
					nofNewTail += nofNewPerProc;
			}
			nReq = 0;
			for(set<int>::iterator sendit = sendersPerProc[rank].begin(); sendit != senditend; ++sendit){
				//nofBytesOverProc += recvBuffers[sit->first].commBufferSize;
				error_flag = MPI_Irecv(recvBuffers[*sendit].commBuffer,recvBuffers[*sendit].commBufferSize,MPI_PACKED,*sendit,rank,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
				error_flag =  MPI_Isend(rsit->second.commBuffer,rsit->second.commBufferSize,MPI_PACKED,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			MPI_Waitall(nReq,req,stats);

			//MOVE RESIDENT TO BEGIN IN OCTANTS
			uint32_t resEnd = octree.getNumOctants() - tailOffset;
			uint32_t nofResidents = resEnd - headOffset;
			int octCounter = 0;
			for(uint32_t i = headOffset; i < resEnd; ++i){
				octree.octants[octCounter] = octree.octants[i];
				++octCounter;
			}
			uint32_t newCounter = nofNewHead + nofNewTail + nofResidents;
			octree.octants.resize(newCounter);
			//MOVE RESIDENTS IN RIGHT POSITION
			uint32_t resCounter = nofNewHead + nofResidents - 1;
			for(uint32_t k = 0; k < nofResidents ; ++k){
				octree.octants[resCounter - k] = octree.octants[nofResidents - k - 1];
			}

			//UNPACK BUFFERS AND BUILD NEW OCTANTS
			newCounter = 0;
			bool jumpResident = false;
			map<int,Class_Comm_Buffer>::iterator rbitend = recvBuffers.end();
			for(map<int,Class_Comm_Buffer>::iterator rbit = recvBuffers.begin(); rbit != rbitend; ++rbit){
				uint32_t nofNewPerProc = (uint32_t)(rbit->second.commBufferSize / (uint32_t)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8)));
				int pos = 0;
				if(rbit->first > rank && !jumpResident){
					newCounter += nofResidents ;
					jumpResident = true;
				}
				for(int i = nofNewPerProc - 1; i >= 0; --i){
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&x,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&y,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&z,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&l,1,MPI_UINT8_T,MPI_COMM_WORLD);
					octree.octants[newCounter] = Class_Octant<3>(l,x,y,z);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&m,1,MPI_INT8_T,MPI_COMM_WORLD);
					octree.octants[newCounter].setMarker(m);
					for(int j = 0; j < 16; ++j){
						error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&info[j],1,MPI::BOOL,MPI_COMM_WORLD);
						octree.octants[newCounter].info[j] = info[j];
					}
					++newCounter;
				}
			}
			octree.octants.shrink_to_fit();
			octree.pborders.clear();

			delete [] newPartitionRangeGlobalidx;
			newPartitionRangeGlobalidx = NULL;

			//Update and ghosts here
			updateLoadBalance();
			setPboundGhosts();

		}
		delete [] partition;
		partition = NULL;

		//Write info of final partition on log
		writeLog(" ");
		writeLog(" Final Parallel partition : ");
		writeLog(" Octants for proc	"+ to_string(0)+"	:	" + to_string(partition_range_globalidx[0]+1));
		for(int ii=1; ii<nproc; ii++){
			writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]-partition_range_globalidx[ii-1]));
		}
		writeLog(" ");
		writeLog("---------------------------------------------");

	};

	//=================================================================================//

	void loadBalance(uint8_t & level){					//assign the octants to the processes following a computed partition with complete families contained in octants of n "level" over the leaf in each process
		//Write info on log
		writeLog("---------------------------------------------");
		writeLog(" LOAD BALANCE ");

		uint32_t* partition = new uint32_t [nproc];
		computePartition(partition, level);
		if(serial)
		{
			writeLog(" ");
			writeLog(" Initial Serial distribution : ");
			for(int ii=0; ii<nproc; ii++){
				writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]+1));
			}

			uint32_t stride = 0;
			for(int i = 0; i < rank; ++i)
				stride += partition[i];
			Class_Local_Tree<3>::OctantsType::const_iterator first = octree.octants.begin() + stride;
			Class_Local_Tree<3>::OctantsType::const_iterator last = first + partition[rank];
			octree.octants.assign(first, last);
			octree.octants.shrink_to_fit();
			first = octree.octants.end();
			last = octree.octants.end();

			//Update and ghosts here
			updateLoadBalance();
			setPboundGhosts();

		}
		else
		{
			writeLog(" ");
			writeLog(" Initial Parallel partition : ");
			writeLog(" Octants for proc	"+ to_string(0)+"	:	" + to_string(partition_range_globalidx[0]+1));
			for(int ii=1; ii<nproc; ii++){
				writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]-partition_range_globalidx[ii-1]));
			}

			//empty ghosts
			octree.ghosts.clear();
			octree.size_ghosts = 0;
			//compute new partition range globalidx
			uint64_t* newPartitionRangeGlobalidx = new uint64_t[nproc];
			for(int p = 0; p < nproc; ++p){
				newPartitionRangeGlobalidx[p] = 0;
				for(int pp = 0; pp <= p; ++pp)
					newPartitionRangeGlobalidx[p] += (uint64_t)partition[pp];
				--newPartitionRangeGlobalidx[p];
			}

			//find resident octants local offset lastHead(lh) and firstTail(ft)
			int32_t lh,ft;
			if(rank == 0)
				lh = -1;
			else{
				lh = (int32_t)(newPartitionRangeGlobalidx[rank-1] + 1 - partition_range_globalidx[rank-1] - 1 - 1);
			}
			if(lh < 0)
				lh = - 1;
			else if(lh > octree.octants.size() - 1)
				lh = octree.octants.size() - 1;

			if(rank == nproc - 1)
				ft = octree.octants.size();
			else if(rank == 0)
				ft = (int32_t)(newPartitionRangeGlobalidx[rank] + 1);
			else{
				ft = (int32_t)(newPartitionRangeGlobalidx[rank] - partition_range_globalidx[rank -1]);
			}
			if(ft > (int32_t)(octree.octants.size() - 1))
				ft = octree.octants.size();
			else if(ft < 0)
				ft = 0;

			//compute size Head and size Tail
			uint32_t headSize = (uint32_t)(lh + 1);
			uint32_t tailSize = (uint32_t)(octree.octants.size() - ft);
			uint32_t headOffset = headSize;
			uint32_t tailOffset = tailSize;

			//build send buffers
			map<int,Class_Comm_Buffer> sendBuffers;

			//Compute first predecessor and first successor to send buffers to
			int64_t firstOctantGlobalIdx = 0;// offset to compute global index of each octant in every process
			int64_t globalLastHead = (int64_t) lh;
			int64_t globalFirstTail = (int64_t) ft; //lastHead and firstTail in global ordering
			int firstPredecessor = -1;
			int firstSuccessor = nproc;
			if(rank != 0){
				firstOctantGlobalIdx = (int64_t)(partition_range_globalidx[rank-1] + 1);
				globalLastHead = firstOctantGlobalIdx + (int64_t)lh;
				globalFirstTail = firstOctantGlobalIdx + (int64_t)ft;
				for(int pre = rank - 1; pre >=0; --pre){
					if((uint64_t)globalLastHead <= newPartitionRangeGlobalidx[pre])
						firstPredecessor = pre;
				}
				for(int post = rank + 1; post < nproc; ++post){
					if((uint64_t)globalFirstTail <= newPartitionRangeGlobalidx[post] && (uint64_t)globalFirstTail > newPartitionRangeGlobalidx[post-1])
						firstSuccessor = post;
				}
			}
			else if(rank == 0){
				firstSuccessor = 1;
			}
			MPI_Barrier(MPI_COMM_WORLD); //da spostare prima della prima comunicazione

			uint32_t x,y,z;
			uint8_t l;
			int8_t m;
			bool info[16];
			//build send buffers from Head
			if(headSize != 0){
				for(int p = firstPredecessor; p >= 0; --p){
					if(headSize <=partition[p]){
						int buffSize = headSize * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						int pos = 0;
						for(uint32_t i = 0; i <= lh; ++i){
							//PACK octants from 0 to lh in sendBuffer[p]
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							}
						}
						break;
					}
					else{
						int buffSize = partition[p] * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						int pos = 0;
						for(uint32_t i = lh - partition[p] + 1; i <= lh; ++i){
							//pack octants from lh - partition[p] to lh
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							}
						}
						lh -= partition[p];
						headSize = lh + 1;
					}
				}

			}
			//build send buffers from Tail
			if(tailSize != 0){
				for(int p = firstSuccessor; p < nproc; ++p){
					if(tailSize <= partition[p]){
						int buffSize = tailSize * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						int pos = 0;
						uint32_t octantsSize = (uint32_t)octree.octants.size();
						for(uint32_t i = ft; i < octantsSize; ++i){
							//PACK octants from ft to octantsSize-1
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							}
						}
						break;
					}
					else{
						int buffSize = partition[p] * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						uint32_t endOctants = ft + partition[p] - 1;
						int pos = 0;
						for(uint32_t i = ft; i <= endOctants; ++i ){
							//PACK octants from ft to ft + partition[p] -1
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
							}
						}
						ft += partition[p];
						tailSize -= partition[p];
					}
				}
			}

			//Build receiver sources
			vector<Class_Array> recvs(nproc);
			recvs[rank] = Class_Array((uint32_t)sendBuffers.size()+1,-1);
			recvs[rank].array[0] = rank;
			int counter = 1;
			map<int,Class_Comm_Buffer>::iterator sitend = sendBuffers.end();
			for(map<int,Class_Comm_Buffer>::iterator sit = sendBuffers.begin(); sit != sitend; ++sit){
				recvs[rank].array[counter] = sit->first;
				++counter;
			}
			int nofRecvsPerProc[nproc];
			error_flag = MPI_Allgather(&recvs[rank].arraySize,1,MPI_INT,nofRecvsPerProc,1,MPI_INT,MPI_COMM_WORLD);
			int globalRecvsBuffSize = 0;
			int displays[nproc];
			for(int pp = 0; pp < nproc; ++pp){
				displays[pp] = 0;
				globalRecvsBuffSize += nofRecvsPerProc[pp];
				for(int ppp = 0; ppp < pp; ++ppp){
					displays[pp] += nofRecvsPerProc[ppp];
				}
			}
			int globalRecvsBuff[globalRecvsBuffSize];
			error_flag = MPI_Allgatherv(recvs[rank].array,recvs[rank].arraySize,MPI_INT,globalRecvsBuff,nofRecvsPerProc,displays,MPI_INT,MPI_COMM_WORLD);

			vector<set<int> > sendersPerProc(nproc);
			for(int pin = 0; pin < nproc; ++pin){
				for(int k = displays[pin]+1; k < displays[pin] + nofRecvsPerProc[pin]; ++k){
					sendersPerProc[globalRecvsBuff[k]].insert(globalRecvsBuff[displays[pin]]);
				}
			}

			//Communicate Octants (size)
			MPI_Request req[sendBuffers.size()+sendersPerProc[rank].size()];
			MPI_Status stats[sendBuffers.size()+sendersPerProc[rank].size()];
			int nReq = 0;
			map<int,int> recvBufferSizePerProc;
			set<int>::iterator senditend = sendersPerProc[rank].end();
			for(set<int>::iterator sendit = sendersPerProc[rank].begin(); sendit != senditend; ++sendit){
				recvBufferSizePerProc[*sendit] = 0;
				error_flag = MPI_Irecv(&recvBufferSizePerProc[*sendit],1,MPI_UINT32_T,*sendit,rank,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			map<int,Class_Comm_Buffer>::reverse_iterator rsitend = sendBuffers.rend();
			for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
				error_flag =  MPI_Isend(&rsit->second.commBufferSize,1,MPI_UINT32_T,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			MPI_Waitall(nReq,req,stats);

			//COMMUNICATE THE BUFFERS TO THE RECEIVERS
			//recvBuffers structure is declared and each buffer is initialized to the right size
			//then, sendBuffers are communicated by senders and stored in recvBuffers in the receivers
			uint32_t nofNewHead = 0;
			uint32_t nofNewTail = 0;
			map<int,Class_Comm_Buffer> recvBuffers;
			map<int,int>::iterator ritend = recvBufferSizePerProc.end();
			for(map<int,int>::iterator rit = recvBufferSizePerProc.begin(); rit != ritend; ++rit){
				recvBuffers[rit->first] = Class_Comm_Buffer(rit->second,'a');
				uint32_t nofNewPerProc = (uint32_t)(rit->second / (uint32_t)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8)));
				if(rit->first < rank)
					nofNewHead += nofNewPerProc;
				else if(rit->first > rank)
					nofNewTail += nofNewPerProc;
			}
			nReq = 0;
			for(set<int>::iterator sendit = sendersPerProc[rank].begin(); sendit != senditend; ++sendit){
				//nofBytesOverProc += recvBuffers[sit->first].commBufferSize;
				error_flag = MPI_Irecv(recvBuffers[*sendit].commBuffer,recvBuffers[*sendit].commBufferSize,MPI_PACKED,*sendit,rank,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
				error_flag =  MPI_Isend(rsit->second.commBuffer,rsit->second.commBufferSize,MPI_PACKED,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			MPI_Waitall(nReq,req,stats);

			//MOVE RESIDENT TO BEGIN IN OCTANTS
			uint32_t resEnd = octree.getNumOctants() - tailOffset;
			uint32_t nofResidents = resEnd - headOffset;
			int octCounter = 0;
			for(uint32_t i = headOffset; i < resEnd; ++i){
				octree.octants[octCounter] = octree.octants[i];
				++octCounter;
			}
			uint32_t newCounter = nofNewHead + nofNewTail + nofResidents;
			octree.octants.resize(newCounter);
			//MOVE RESIDENTS IN RIGHT POSITION
			uint32_t resCounter = nofNewHead + nofResidents - 1;
			for(uint32_t k = 0; k < nofResidents ; ++k){
				octree.octants[resCounter - k] = octree.octants[nofResidents - k - 1];
			}

			//UNPACK BUFFERS AND BUILD NEW OCTANTS
			newCounter = 0;
			bool jumpResident = false;
			map<int,Class_Comm_Buffer>::iterator rbitend = recvBuffers.end();
			for(map<int,Class_Comm_Buffer>::iterator rbit = recvBuffers.begin(); rbit != rbitend; ++rbit){
				uint32_t nofNewPerProc = (uint32_t)(rbit->second.commBufferSize / (uint32_t)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8)));
				int pos = 0;
				if(rbit->first > rank && !jumpResident){
					newCounter += nofResidents ;
					jumpResident = true;
				}
				for(int i = nofNewPerProc - 1; i >= 0; --i){
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&x,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&y,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&z,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&l,1,MPI_UINT8_T,MPI_COMM_WORLD);
					octree.octants[newCounter] = Class_Octant<3>(l,x,y,z);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&m,1,MPI_INT8_T,MPI_COMM_WORLD);
					octree.octants[newCounter].setMarker(m);
					for(int j = 0; j < 16; ++j){
						error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&pos,&info[j],1,MPI::BOOL,MPI_COMM_WORLD);
						octree.octants[newCounter].info[j] = info[j];
					}
					++newCounter;
				}
			}
			octree.octants.shrink_to_fit();
			octree.pborders.clear();

			delete [] newPartitionRangeGlobalidx;
			newPartitionRangeGlobalidx = NULL;

			//Update and ghosts here
			updateLoadBalance();
			setPboundGhosts();

		}
		delete [] partition;
		partition = NULL;

		//Write info of final partition on log
		writeLog(" ");
		writeLog(" Final Parallel partition : ");
		writeLog(" Octants for proc	"+ to_string(0)+"	:	" + to_string(partition_range_globalidx[0]+1));
		for(int ii=1; ii<nproc; ii++){
			writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]-partition_range_globalidx[ii-1]));
		}
		writeLog(" ");
		writeLog("---------------------------------------------");



	};

	//=================================================================================//

	template<class UserDataComm>
	void loadBalance(UserDataComm & userData){
		//Write info on log
		writeLog("---------------------------------------------");
		writeLog(" LOAD BALANCE ");

		uint32_t* partition = new uint32_t [nproc];
		computePartition(partition);
		if(serial)
		{
			writeLog(" ");
			writeLog(" Initial Serial distribution : ");
			for(int ii=0; ii<nproc; ii++){
				writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]+1));
			}

			uint32_t stride = 0;
			for(int i = 0; i < rank; ++i)
				stride += partition[i];
			Class_Local_Tree<3>::OctantsType::const_iterator first = octree.octants.begin() + stride;
			Class_Local_Tree<3>::OctantsType::const_iterator last = first + partition[rank];
			typename UserDataComm::Data::iterator firstData = userData.data.begin() + stride;
			typename UserDataComm::Data::iterator lastData = firstData + partition[rank];
			octree.octants.assign(first, last);
			userData.data.assign(firstData,lastData);
			octree.octants.shrink_to_fit();
			userData.data.shrink_to_fit();
			first = octree.octants.end();
			last = octree.octants.end();

			//Update and build ghosts here
			updateLoadBalance();
			setPboundGhosts();

			//		userData.ghostData.resize(octree.size_ghosts,0.0);
			//		communicate(userData);

		}
		else
		{
			writeLog(" ");
			writeLog(" Initial Parallel partition : ");
			writeLog(" Octants for proc	"+ to_string(0)+"	:	" + to_string(partition_range_globalidx[0]+1));
			for(int ii=1; ii<nproc; ii++){
				writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]-partition_range_globalidx[ii-1]));
			}

			//empty ghosts
			octree.ghosts.clear();
			octree.size_ghosts = 0;
			//compute new partition range globalidx
			uint64_t* newPartitionRangeGlobalidx = new uint64_t[nproc];
			for(int p = 0; p < nproc; ++p){
				newPartitionRangeGlobalidx[p] = 0;
				for(int pp = 0; pp <= p; ++pp)
					newPartitionRangeGlobalidx[p] += (uint64_t)partition[pp];
				--newPartitionRangeGlobalidx[p];
			}

			//find resident octants local offset lastHead(lh) and firstTail(ft)
			int32_t lh,ft;
			if(rank == 0)
				lh = -1;
			else{
				lh = (int32_t)(newPartitionRangeGlobalidx[rank-1] + 1 - partition_range_globalidx[rank-1] - 1 - 1);
			}
			if(lh < 0)
				lh = - 1;
			else if(lh > octree.octants.size() - 1)
				lh = octree.octants.size() - 1;

			if(rank == nproc - 1)
				ft = octree.octants.size();
			else if(rank == 0)
				ft = (int32_t)(newPartitionRangeGlobalidx[rank] + 1);
			else{
				ft = (int32_t)(newPartitionRangeGlobalidx[rank] - partition_range_globalidx[rank -1]);
			}
			if(ft > (int32_t)(octree.octants.size() - 1))
				ft = octree.octants.size();
			else if(ft < 0)
				ft = 0;

			//compute size Head and size Tail
			uint32_t headSize = (uint32_t)(lh + 1);
			uint32_t tailSize = (uint32_t)(octree.octants.size() - ft);
			uint32_t headOffset = headSize;
			uint32_t tailOffset = tailSize;

			//build send buffers
			map<int,Class_Comm_Buffer> sendBuffers;

			//Compute first predecessor and first successor to send buffers to
			int64_t firstOctantGlobalIdx = 0;// offset to compute global index of each octant in every process
			int64_t globalLastHead = (int64_t) lh;
			int64_t globalFirstTail = (int64_t) ft; //lastHead and firstTail in global ordering
			int firstPredecessor = -1;
			int firstSuccessor = nproc;
			if(rank != 0){
				firstOctantGlobalIdx = (int64_t)(partition_range_globalidx[rank-1] + 1);
				globalLastHead = firstOctantGlobalIdx + (int64_t)lh;
				globalFirstTail = firstOctantGlobalIdx + (int64_t)ft;
				for(int pre = rank - 1; pre >=0; --pre){
					if((uint64_t)globalLastHead <= newPartitionRangeGlobalidx[pre])
						firstPredecessor = pre;
				}
				for(int post = rank + 1; post < nproc; ++post){
					if((uint64_t)globalFirstTail <= newPartitionRangeGlobalidx[post] && (uint64_t)globalFirstTail > newPartitionRangeGlobalidx[post-1])
						firstSuccessor = post;
				}
			}
			else if(rank == 0){
				firstSuccessor = 1;
			}
			MPI_Barrier(MPI_COMM_WORLD); //da spostare prima della prima comunicazione

			uint32_t x,y,z;
			uint8_t l;
			int8_t m;
			bool info[16];
			//build send buffers from Head
			if(headSize != 0){
				for(int p = firstPredecessor; p >= 0; --p){
					if(headSize <=partition[p]){
						int buffSize = headSize * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						//TODO loop over head octants and add data size to buffer size - DONE
						//compute size of data in buffers
						if(userData.fixedSize()){
							buffSize +=  userData.fixedSize() * headSize;
						}
						else{
							for(uint32_t i = 0; i <= lh; ++i){
								buffSize += userData.size(i);
							}
						}
						//add room for int, number of octants in this buffer
						buffSize += sizeof(int);
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						//store the number of octants at the beginning of the buffer
						MPI_Pack(&headSize,1,MPI_UINT32_T,sendBuffers[p].commBuffer,sendBuffers[p].commBufferSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
						//USE BUFFER POS
						//int pos = 0;
						for(uint32_t i = 0; i <= lh; ++i){
							//PACK octants from 0 to lh in sendBuffer[p]
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);

							}
							//TODO call gather to pack user data - DONE
							userData.gather(sendBuffers[p],i);
						}
						break;
					}
					else{
						int buffSize = partition[p] * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						//TODO loop over head octants and add data size to buffer size - DONE
						//compute size of data in buffers
						if(userData.fixedSize()){
							buffSize +=  userData.fixedSize() * partition[p];
						}
						else{
							for(uint32_t i = lh - partition[p] + 1; i <= lh; ++i){
								buffSize += userData.size(i);
							}
						}
						//add room for int, number of octants in this buffer
						buffSize += sizeof(int);
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						//store the number of octants at the beginning of the buffer
						MPI_Pack(&partition[p],1,MPI_UINT32_T,sendBuffers[p].commBuffer,sendBuffers[p].commBufferSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
						//USE BUFFER POS
						//int pos = 0;
						for(uint32_t i = lh - partition[p] + 1; i <= lh; ++i){
							//pack octants from lh - partition[p] to lh
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							}
							//TODO call gather to pack user data - DONE
							userData.gather(sendBuffers[p],i);
						}
						lh -= partition[p];
						headSize = lh + 1;
					}
				}

			}
			//build send buffers from Tail
			if(tailSize != 0){
				for(int p = firstSuccessor; p < nproc; ++p){
					if(tailSize <= partition[p]){
						uint32_t octantsSize = (uint32_t)octree.octants.size();
						int buffSize = tailSize * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						//TODO loop over head octants and add data size to buffer size - DONE
						//compute size of data in buffers
						if(userData.fixedSize()){
							buffSize +=  userData.fixedSize() * tailSize;
						}
						else{
							for(uint32_t i = ft; i <= octantsSize; ++i){
								buffSize += userData.size(i);
							}
						}
						//add room for int, number of octants in this buffer
						buffSize += sizeof(int);
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						//store the number of octants at the beginning of the buffer
						MPI_Pack(&tailSize,1,MPI_UINT32_T,sendBuffers[p].commBuffer,sendBuffers[p].commBufferSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
						//USE BUFFER POS
						//int pos = 0;
						for(uint32_t i = ft; i < octantsSize; ++i){
							//PACK octants from ft to octantsSize-1
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							}
							//TODO call gather to pack user data - DONE
							userData.gather(sendBuffers[p],i);
						}
						break;
					}
					else{
						uint32_t endOctants = ft + partition[p] - 1;
						int buffSize = partition[p] * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						//TODO loop over head octants and add data size to buffer size - DONE
						//compute size of data in buffers
						if(userData.fixedSize()){
							buffSize +=  userData.fixedSize() * partition[p];
						}
						else{
							for(uint32_t i = ft; i <= endOctants; ++i){
								buffSize += userData.size(i);
							}
						}
						//add room for int, number of octants in this buffer
						buffSize += sizeof(int);
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						//store the number of octants at the beginning of the buffer
						MPI_Pack(&partition[p],1,MPI_UINT32_T,sendBuffers[p].commBuffer,sendBuffers[p].commBufferSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
						//int pos = 0;
						for(uint32_t i = ft; i <= endOctants; ++i ){
							//PACK octants from ft to ft + partition[p] -1
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							}
							//TODO call gather to pack user data - DONE
							userData.gather(sendBuffers[p],i);
						}
						ft += partition[p];
						tailSize -= partition[p];
					}
				}
			}

			//Build receiver sources
			vector<Class_Array> recvs(nproc);
			recvs[rank] = Class_Array((uint32_t)sendBuffers.size()+1,-1);
			recvs[rank].array[0] = rank;
			int counter = 1;
			map<int,Class_Comm_Buffer>::iterator sitend = sendBuffers.end();
			for(map<int,Class_Comm_Buffer>::iterator sit = sendBuffers.begin(); sit != sitend; ++sit){
				recvs[rank].array[counter] = sit->first;
				++counter;
			}
			int nofRecvsPerProc[nproc];
			error_flag = MPI_Allgather(&recvs[rank].arraySize,1,MPI_INT,nofRecvsPerProc,1,MPI_INT,MPI_COMM_WORLD);
			int globalRecvsBuffSize = 0;
			int displays[nproc];
			for(int pp = 0; pp < nproc; ++pp){
				displays[pp] = 0;
				globalRecvsBuffSize += nofRecvsPerProc[pp];
				for(int ppp = 0; ppp < pp; ++ppp){
					displays[pp] += nofRecvsPerProc[ppp];
				}
			}
			int globalRecvsBuff[globalRecvsBuffSize];
			error_flag = MPI_Allgatherv(recvs[rank].array,recvs[rank].arraySize,MPI_INT,globalRecvsBuff,nofRecvsPerProc,displays,MPI_INT,MPI_COMM_WORLD);

			vector<set<int> > sendersPerProc(nproc);
			for(int pin = 0; pin < nproc; ++pin){
				for(int k = displays[pin]+1; k < displays[pin] + nofRecvsPerProc[pin]; ++k){
					sendersPerProc[globalRecvsBuff[k]].insert(globalRecvsBuff[displays[pin]]);
				}
			}

			//Communicate Octants (size)
			MPI_Request req[sendBuffers.size()+sendersPerProc[rank].size()];
			MPI_Status stats[sendBuffers.size()+sendersPerProc[rank].size()];
			int nReq = 0;
			map<int,int> recvBufferSizePerProc;
			set<int>::iterator senditend = sendersPerProc[rank].end();
			for(set<int>::iterator sendit = sendersPerProc[rank].begin(); sendit != senditend; ++sendit){
				recvBufferSizePerProc[*sendit] = 0;
				error_flag = MPI_Irecv(&recvBufferSizePerProc[*sendit],1,MPI_UINT32_T,*sendit,rank,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			map<int,Class_Comm_Buffer>::reverse_iterator rsitend = sendBuffers.rend();
			for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
				error_flag =  MPI_Isend(&rsit->second.commBufferSize,1,MPI_UINT32_T,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			MPI_Waitall(nReq,req,stats);

			//COMMUNICATE THE BUFFERS TO THE RECEIVERS
			//recvBuffers structure is declared and each buffer is initialized to the right size
			//then, sendBuffers are communicated by senders and stored in recvBuffers in the receivers
			uint32_t nofNewHead = 0;
			uint32_t nofNewTail = 0;
			map<int,Class_Comm_Buffer> recvBuffers;

			map<int,int>::iterator ritend = recvBufferSizePerProc.end();
			for(map<int,int>::iterator rit = recvBufferSizePerProc.begin(); rit != ritend; ++rit){
				recvBuffers[rit->first] = Class_Comm_Buffer(rit->second,'a');
				//			uint32_t nofNewPerProc = (uint32_t)(rit->second / (uint32_t)ceil((double)octantBytes / (double)(CHAR_BIT/8)));
				//			if(rit->first < rank)
				//				nofNewHead += nofNewPerProc;
				//			else if(rit->first > rank)
				//				nofNewTail += nofNewPerProc;
			}

			nReq = 0;
			for(set<int>::iterator sendit = sendersPerProc[rank].begin(); sendit != senditend; ++sendit){
				//nofBytesOverProc += recvBuffers[sit->first].commBufferSize;
				error_flag = MPI_Irecv(recvBuffers[*sendit].commBuffer,recvBuffers[*sendit].commBufferSize,MPI_PACKED,*sendit,rank,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
				error_flag =  MPI_Isend(rsit->second.commBuffer,rsit->second.commBufferSize,MPI_PACKED,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			MPI_Waitall(nReq,req,stats);

			//Unpack number of octants per sender
			map<int,uint32_t> nofNewOverProcs;
			map<int,Class_Comm_Buffer>::iterator rbitend = recvBuffers.end();
			for(map<int,Class_Comm_Buffer>::iterator rbit = recvBuffers.begin(); rbit != rbitend; ++rbit){
				uint32_t nofNewPerProc;
				MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&nofNewPerProc,1,MPI_UINT32_T,MPI_COMM_WORLD);
				nofNewOverProcs[rbit->first] = nofNewPerProc;
				if(rbit->first < rank)
					nofNewHead += nofNewPerProc;
				else if(rbit->first > rank)
					nofNewTail += nofNewPerProc;
			}

			//MOVE RESIDENT TO BEGIN IN OCTANTS
			uint32_t resEnd = octree.getNumOctants() - tailOffset;
			uint32_t nofResidents = resEnd - headOffset;
			uint32_t octCounter = 0;
			for(uint32_t i = headOffset; i < resEnd; ++i){
				octree.octants[octCounter] = octree.octants[i];
				//TODO move data - DONE
				userData.move(i,octCounter);
				++octCounter;
			}
			uint32_t newCounter = nofNewHead + nofNewTail + nofResidents;
			octree.octants.resize(newCounter);
			userData.data.resize(newCounter);
			//MOVE RESIDENTS IN RIGHT POSITION
			uint32_t resCounter = nofNewHead + nofResidents - 1;
			for(uint32_t k = 0; k < nofResidents ; ++k){
				octree.octants[resCounter - k] = octree.octants[nofResidents - k - 1];
				//TODO move data - DON
				userData.move(nofResidents - k - 1,resCounter - k);
			}

			//UNPACK BUFFERS AND BUILD NEW OCTANTS
			newCounter = 0;
			bool jumpResident = false;

			for(map<int,Class_Comm_Buffer>::iterator rbit = recvBuffers.begin(); rbit != rbitend; ++rbit){
				//TODO change new octants counting, probably you have to communicate the number of news per proc
				uint32_t nofNewPerProc = nofNewOverProcs[rbit->first];//(uint32_t)(rbit->second.commBufferSize / (uint32_t)ceil((double)octantBytes / (double)(CHAR_BIT/8)));
				//int pos = 0;
				if(rbit->first > rank && !jumpResident){
					newCounter += nofResidents ;
					jumpResident = true;
				}
				for(int i = nofNewPerProc - 1; i >= 0; --i){
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&x,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&y,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&z,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&l,1,MPI_UINT8_T,MPI_COMM_WORLD);
					octree.octants[newCounter] = Class_Octant<3>(l,x,y,z);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&m,1,MPI_INT8_T,MPI_COMM_WORLD);
					octree.octants[newCounter].setMarker(m);
					for(int j = 0; j < 16; ++j){
						error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&info[j],1,MPI::BOOL,MPI_COMM_WORLD);
						octree.octants[newCounter].info[j] = info[j];
					}
					//TODO Unpack data
					userData.scatter(rbit->second,newCounter);
					++newCounter;
				}
			}
			octree.octants.shrink_to_fit();
			userData.data.shrink_to_fit();
			octree.pborders.clear();

			delete [] newPartitionRangeGlobalidx;
			newPartitionRangeGlobalidx = NULL;

			//Update and ghosts here
			updateLoadBalance();
			setPboundGhosts();

		}
		delete [] partition;
		partition = NULL;

		//Write info of final partition on log
		writeLog(" ");
		writeLog(" Final Parallel partition : ");
		writeLog(" Octants for proc	"+ to_string(0)+"	:	" + to_string(partition_range_globalidx[0]+1));
		for(int ii=1; ii<nproc; ii++){
			writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]-partition_range_globalidx[ii-1]));
		}
		writeLog(" ");
		writeLog("---------------------------------------------");

	};

	//=================================================================================//

	template<class UserDataComm>
	void loadBalance(UserDataComm & userData, uint8_t & level){
		//Write info on log
		writeLog("---------------------------------------------");
		writeLog(" LOAD BALANCE ");

		uint32_t* partition = new uint32_t [nproc];
		computePartition(partition,level);
		if(serial)
		{
			writeLog(" ");
			writeLog(" Initial Serial distribution : ");
			for(int ii=0; ii<nproc; ii++){
				writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]+1));
			}

			uint32_t stride = 0;
			for(int i = 0; i < rank; ++i)
				stride += partition[i];
			Class_Local_Tree<3>::OctantsType::const_iterator first = octree.octants.begin() + stride;
			Class_Local_Tree<3>::OctantsType::const_iterator last = first + partition[rank];
			typename UserDataComm::Data::iterator firstData = userData.data.begin() + stride;
			typename UserDataComm::Data::iterator lastData = firstData + partition[rank];
			octree.octants.assign(first, last);
			userData.data.assign(firstData,lastData);
			octree.octants.shrink_to_fit();
			userData.data.shrink_to_fit();
			first = octree.octants.end();
			last = octree.octants.end();

			//Update and build ghosts here
			updateLoadBalance();
			setPboundGhosts();

			//		userData.ghostData.resize(octree.size_ghosts,0.0);
			//		communicate(userData);

		}
		else
		{
			writeLog(" ");
			writeLog(" Initial Parallel partition : ");
			writeLog(" Octants for proc	"+ to_string(0)+"	:	" + to_string(partition_range_globalidx[0]+1));
			for(int ii=1; ii<nproc; ii++){
				writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]-partition_range_globalidx[ii-1]));
			}

			//empty ghosts
			octree.ghosts.clear();
			octree.size_ghosts = 0;
			//compute new partition range globalidx
			uint64_t* newPartitionRangeGlobalidx = new uint64_t[nproc];
			for(int p = 0; p < nproc; ++p){
				newPartitionRangeGlobalidx[p] = 0;
				for(int pp = 0; pp <= p; ++pp)
					newPartitionRangeGlobalidx[p] += (uint64_t)partition[pp];
				--newPartitionRangeGlobalidx[p];
			}

			//find resident octants local offset lastHead(lh) and firstTail(ft)
			int32_t lh,ft;
			if(rank == 0)
				lh = -1;
			else{
				lh = (int32_t)(newPartitionRangeGlobalidx[rank-1] + 1 - partition_range_globalidx[rank-1] - 1 - 1);
			}
			if(lh < 0)
				lh = - 1;
			else if(lh > octree.octants.size() - 1)
				lh = octree.octants.size() - 1;

			if(rank == nproc - 1)
				ft = octree.octants.size();
			else if(rank == 0)
				ft = (int32_t)(newPartitionRangeGlobalidx[rank] + 1);
			else{
				ft = (int32_t)(newPartitionRangeGlobalidx[rank] - partition_range_globalidx[rank -1]);
			}
			if(ft > (int32_t)(octree.octants.size() - 1))
				ft = octree.octants.size();
			else if(ft < 0)
				ft = 0;

			//compute size Head and size Tail
			uint32_t headSize = (uint32_t)(lh + 1);
			uint32_t tailSize = (uint32_t)(octree.octants.size() - ft);
			uint32_t headOffset = headSize;
			uint32_t tailOffset = tailSize;

			//build send buffers
			map<int,Class_Comm_Buffer> sendBuffers;

			//Compute first predecessor and first successor to send buffers to
			int64_t firstOctantGlobalIdx = 0;// offset to compute global index of each octant in every process
			int64_t globalLastHead = (int64_t) lh;
			int64_t globalFirstTail = (int64_t) ft; //lastHead and firstTail in global ordering
			int firstPredecessor = -1;
			int firstSuccessor = nproc;
			if(rank != 0){
				firstOctantGlobalIdx = (int64_t)(partition_range_globalidx[rank-1] + 1);
				globalLastHead = firstOctantGlobalIdx + (int64_t)lh;
				globalFirstTail = firstOctantGlobalIdx + (int64_t)ft;
				for(int pre = rank - 1; pre >=0; --pre){
					if((uint64_t)globalLastHead <= newPartitionRangeGlobalidx[pre])
						firstPredecessor = pre;
				}
				for(int post = rank + 1; post < nproc; ++post){
					if((uint64_t)globalFirstTail <= newPartitionRangeGlobalidx[post] && (uint64_t)globalFirstTail > newPartitionRangeGlobalidx[post-1])
						firstSuccessor = post;
				}
			}
			else if(rank == 0){
				firstSuccessor = 1;
			}
			MPI_Barrier(MPI_COMM_WORLD); //da spostare prima della prima comunicazione

			uint32_t x,y,z;
			uint8_t l;
			int8_t m;
			bool info[16];
			//build send buffers from Head
			if(headSize != 0){
				for(int p = firstPredecessor; p >= 0; --p){
					if(headSize <=partition[p]){
						int buffSize = headSize * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						//TODO loop over head octants and add data size to buffer size - DONE
						//compute size of data in buffers
						if(userData.fixedSize()){
							buffSize +=  userData.fixedSize() * headSize;
						}
						else{
							for(uint32_t i = 0; i <= lh; ++i){
								buffSize += userData.size(i);
							}
						}
						//add room for int, number of octants in this buffer
						buffSize += sizeof(int);
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						//store the number of octants at the beginning of the buffer
						MPI_Pack(&headSize,1,MPI_UINT32_T,sendBuffers[p].commBuffer,sendBuffers[p].commBufferSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
						//USE BUFFER POS
						//int pos = 0;
						for(uint32_t i = 0; i <= lh; ++i){
							//PACK octants from 0 to lh in sendBuffer[p]
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);

							}
							//TODO call gather to pack user data - DONE
							userData.gather(sendBuffers[p],i);
						}
						break;
					}
					else{
						int buffSize = partition[p] * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						//TODO loop over head octants and add data size to buffer size - DONE
						//compute size of data in buffers
						if(userData.fixedSize()){
							buffSize +=  userData.fixedSize() * partition[p];
						}
						else{
							for(uint32_t i = lh - partition[p] + 1; i <= lh; ++i){
								buffSize += userData.size(i);
							}
						}
						//add room for int, number of octants in this buffer
						buffSize += sizeof(int);
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						//store the number of octants at the beginning of the buffer
						MPI_Pack(&partition[p],1,MPI_UINT32_T,sendBuffers[p].commBuffer,sendBuffers[p].commBufferSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
						//USE BUFFER POS
						//int pos = 0;
						for(uint32_t i = lh - partition[p] + 1; i <= lh; ++i){
							//pack octants from lh - partition[p] to lh
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							}
							//TODO call gather to pack user data - DONE
							userData.gather(sendBuffers[p],i);
						}
						lh -= partition[p];
						headSize = lh + 1;
					}
				}

			}
			//build send buffers from Tail
			if(tailSize != 0){
				for(int p = firstSuccessor; p < nproc; ++p){
					if(tailSize <= partition[p]){
						uint32_t octantsSize = (uint32_t)octree.octants.size();
						int buffSize = tailSize * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						//TODO loop over head octants and add data size to buffer size - DONE
						//compute size of data in buffers
						if(userData.fixedSize()){
							buffSize +=  userData.fixedSize() * tailSize;
						}
						else{
							for(uint32_t i = ft; i <= octantsSize; ++i){
								buffSize += userData.size(i);
							}
						}
						//add room for int, number of octants in this buffer
						buffSize += sizeof(int);
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						//store the number of octants at the beginning of the buffer
						MPI_Pack(&tailSize,1,MPI_UINT32_T,sendBuffers[p].commBuffer,sendBuffers[p].commBufferSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
						//USE BUFFER POS
						//int pos = 0;
						for(uint32_t i = ft; i < octantsSize; ++i){
							//PACK octants from ft to octantsSize-1
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							}
							//TODO call gather to pack user data - DONE
							userData.gather(sendBuffers[p],i);
						}
						break;
					}
					else{
						uint32_t endOctants = ft + partition[p] - 1;
						int buffSize = partition[p] * (int)ceil((double)global3D.octantBytes / (double)(CHAR_BIT/8));
						//TODO loop over head octants and add data size to buffer size - DONE
						//compute size of data in buffers
						if(userData.fixedSize()){
							buffSize +=  userData.fixedSize() * partition[p];
						}
						else{
							for(uint32_t i = ft; i <= endOctants; ++i){
								buffSize += userData.size(i);
							}
						}
						//add room for int, number of octants in this buffer
						buffSize += sizeof(int);
						sendBuffers[p] = Class_Comm_Buffer(buffSize,'a');
						//store the number of octants at the beginning of the buffer
						MPI_Pack(&partition[p],1,MPI_UINT32_T,sendBuffers[p].commBuffer,sendBuffers[p].commBufferSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
						//int pos = 0;
						for(uint32_t i = ft; i <= endOctants; ++i ){
							//PACK octants from ft to ft + partition[p] -1
							const Class_Octant<3> & octant = octree.octants[i];
							x = octant.getX();
							y = octant.getY();
							z = octant.getZ();
							l = octant.getLevel();
							m = octant.getMarker();
							memcpy(info,octant.info,16);
							error_flag = MPI_Pack(&x,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&y,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&z,1,MPI_UINT32_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&l,1,MPI_UINT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							error_flag = MPI_Pack(&m,1,MPI_INT8_T,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							for(int j = 0; j < 16; ++j){
								MPI_Pack(&info[j],1,MPI::BOOL,sendBuffers[p].commBuffer,buffSize,&sendBuffers[p].pos,MPI_COMM_WORLD);
							}
							//TODO call gather to pack user data - DONE
							userData.gather(sendBuffers[p],i);
						}
						ft += partition[p];
						tailSize -= partition[p];
					}
				}
			}

			//Build receiver sources
			vector<Class_Array> recvs(nproc);
			recvs[rank] = Class_Array((uint32_t)sendBuffers.size()+1,-1);
			recvs[rank].array[0] = rank;
			int counter = 1;
			map<int,Class_Comm_Buffer>::iterator sitend = sendBuffers.end();
			for(map<int,Class_Comm_Buffer>::iterator sit = sendBuffers.begin(); sit != sitend; ++sit){
				recvs[rank].array[counter] = sit->first;
				++counter;
			}
			int nofRecvsPerProc[nproc];
			error_flag = MPI_Allgather(&recvs[rank].arraySize,1,MPI_INT,nofRecvsPerProc,1,MPI_INT,MPI_COMM_WORLD);
			int globalRecvsBuffSize = 0;
			int displays[nproc];
			for(int pp = 0; pp < nproc; ++pp){
				displays[pp] = 0;
				globalRecvsBuffSize += nofRecvsPerProc[pp];
				for(int ppp = 0; ppp < pp; ++ppp){
					displays[pp] += nofRecvsPerProc[ppp];
				}
			}
			int globalRecvsBuff[globalRecvsBuffSize];
			error_flag = MPI_Allgatherv(recvs[rank].array,recvs[rank].arraySize,MPI_INT,globalRecvsBuff,nofRecvsPerProc,displays,MPI_INT,MPI_COMM_WORLD);

			vector<set<int> > sendersPerProc(nproc);
			for(int pin = 0; pin < nproc; ++pin){
				for(int k = displays[pin]+1; k < displays[pin] + nofRecvsPerProc[pin]; ++k){
					sendersPerProc[globalRecvsBuff[k]].insert(globalRecvsBuff[displays[pin]]);
				}
			}

			//Communicate Octants (size)
			MPI_Request req[sendBuffers.size()+sendersPerProc[rank].size()];
			MPI_Status stats[sendBuffers.size()+sendersPerProc[rank].size()];
			int nReq = 0;
			map<int,int> recvBufferSizePerProc;
			set<int>::iterator senditend = sendersPerProc[rank].end();
			for(set<int>::iterator sendit = sendersPerProc[rank].begin(); sendit != senditend; ++sendit){
				recvBufferSizePerProc[*sendit] = 0;
				error_flag = MPI_Irecv(&recvBufferSizePerProc[*sendit],1,MPI_UINT32_T,*sendit,rank,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			map<int,Class_Comm_Buffer>::reverse_iterator rsitend = sendBuffers.rend();
			for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
				error_flag =  MPI_Isend(&rsit->second.commBufferSize,1,MPI_UINT32_T,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			MPI_Waitall(nReq,req,stats);

			//COMMUNICATE THE BUFFERS TO THE RECEIVERS
			//recvBuffers structure is declared and each buffer is initialized to the right size
			//then, sendBuffers are communicated by senders and stored in recvBuffers in the receivers
			uint32_t nofNewHead = 0;
			uint32_t nofNewTail = 0;
			map<int,Class_Comm_Buffer> recvBuffers;

			map<int,int>::iterator ritend = recvBufferSizePerProc.end();
			for(map<int,int>::iterator rit = recvBufferSizePerProc.begin(); rit != ritend; ++rit){
				recvBuffers[rit->first] = Class_Comm_Buffer(rit->second,'a');
				//			uint32_t nofNewPerProc = (uint32_t)(rit->second / (uint32_t)ceil((double)octantBytes / (double)(CHAR_BIT/8)));
				//			if(rit->first < rank)
				//				nofNewHead += nofNewPerProc;
				//			else if(rit->first > rank)
				//				nofNewTail += nofNewPerProc;
			}

			nReq = 0;
			for(set<int>::iterator sendit = sendersPerProc[rank].begin(); sendit != senditend; ++sendit){
				//nofBytesOverProc += recvBuffers[sit->first].commBufferSize;
				error_flag = MPI_Irecv(recvBuffers[*sendit].commBuffer,recvBuffers[*sendit].commBufferSize,MPI_PACKED,*sendit,rank,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
				error_flag =  MPI_Isend(rsit->second.commBuffer,rsit->second.commBufferSize,MPI_PACKED,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
				++nReq;
			}
			MPI_Waitall(nReq,req,stats);

			//Unpack number of octants per sender
			map<int,uint32_t> nofNewOverProcs;
			map<int,Class_Comm_Buffer>::iterator rbitend = recvBuffers.end();
			for(map<int,Class_Comm_Buffer>::iterator rbit = recvBuffers.begin(); rbit != rbitend; ++rbit){
				uint32_t nofNewPerProc;
				MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&nofNewPerProc,1,MPI_UINT32_T,MPI_COMM_WORLD);
				nofNewOverProcs[rbit->first] = nofNewPerProc;
				if(rbit->first < rank)
					nofNewHead += nofNewPerProc;
				else if(rbit->first > rank)
					nofNewTail += nofNewPerProc;
			}

			//MOVE RESIDENT TO BEGIN IN OCTANTS
			uint32_t resEnd = octree.getNumOctants() - tailOffset;
			uint32_t nofResidents = resEnd - headOffset;
			uint32_t octCounter = 0;
			for(uint32_t i = headOffset; i < resEnd; ++i){
				octree.octants[octCounter] = octree.octants[i];
				//TODO move data - DONE
				userData.move(i,octCounter);
				++octCounter;
			}
			uint32_t newCounter = nofNewHead + nofNewTail + nofResidents;
			octree.octants.resize(newCounter);
			userData.data.resize(newCounter);
			//MOVE RESIDENTS IN RIGHT POSITION
			uint32_t resCounter = nofNewHead + nofResidents - 1;
			for(uint32_t k = 0; k < nofResidents ; ++k){
				octree.octants[resCounter - k] = octree.octants[nofResidents - k - 1];
				//TODO move data - DON
				userData.move(nofResidents - k - 1,resCounter - k);
			}

			//UNPACK BUFFERS AND BUILD NEW OCTANTS
			newCounter = 0;
			bool jumpResident = false;

			for(map<int,Class_Comm_Buffer>::iterator rbit = recvBuffers.begin(); rbit != rbitend; ++rbit){
				//TODO change new octants counting, probably you have to communicate the number of news per proc
				uint32_t nofNewPerProc = nofNewOverProcs[rbit->first];//(uint32_t)(rbit->second.commBufferSize / (uint32_t)ceil((double)octantBytes / (double)(CHAR_BIT/8)));
				//int pos = 0;
				if(rbit->first > rank && !jumpResident){
					newCounter += nofResidents ;
					jumpResident = true;
				}
				for(int i = nofNewPerProc - 1; i >= 0; --i){
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&x,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&y,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&z,1,MPI_UINT32_T,MPI_COMM_WORLD);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&l,1,MPI_UINT8_T,MPI_COMM_WORLD);
					octree.octants[newCounter] = Class_Octant<3>(l,x,y,z);
					error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&m,1,MPI_INT8_T,MPI_COMM_WORLD);
					octree.octants[newCounter].setMarker(m);
					for(int j = 0; j < 16; ++j){
						error_flag = MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&info[j],1,MPI::BOOL,MPI_COMM_WORLD);
						octree.octants[newCounter].info[j] = info[j];
					}
					//TODO Unpack data
					userData.scatter(rbit->second,newCounter);
					++newCounter;
				}
			}
			octree.octants.shrink_to_fit();
			userData.data.shrink_to_fit();
			octree.pborders.clear();

			delete [] newPartitionRangeGlobalidx;
			newPartitionRangeGlobalidx = NULL;

			//Update and ghosts here
			updateLoadBalance();
			setPboundGhosts();

		}
		delete [] partition;
		partition = NULL;

		//Write info of final partition on log
		writeLog(" ");
		writeLog(" Final Parallel partition : ");
		writeLog(" Octants for proc	"+ to_string(0)+"	:	" + to_string(partition_range_globalidx[0]+1));
		for(int ii=1; ii<nproc; ii++){
			writeLog(" Octants for proc	"+ to_string(ii)+"	:	" + to_string(partition_range_globalidx[ii]-partition_range_globalidx[ii-1]));
		}
		writeLog(" ");
		writeLog("---------------------------------------------");

	};

	//=================================================================================//

	void updateAdapt(){									//update Class_Para_Tree members after a refine and/or coarse
		if(serial)
		{
			max_depth = octree.local_max_depth;
			global_num_octants = octree.getNumOctants();
			for(int p = 0; p < nproc; ++p){
				partition_range_globalidx[p] = global_num_octants - 1;
				//partition_range_position[p] = octree.last_desc.computeMorton();
			}
		}
		else
		{
			//update max_depth
			error_flag = MPI_Allreduce(&octree.local_max_depth,&max_depth,1,MPI_UINT8_T,MPI_MAX,MPI_COMM_WORLD);
			//update global_num_octants
			uint64_t local_num_octants = octree.getNumOctants();
			error_flag = MPI_Allreduce(&local_num_octants,&global_num_octants,1,MPI_UINT64_T,MPI_SUM,MPI_COMM_WORLD);
			//update partition_range_globalidx
			uint64_t rbuff [nproc];
			error_flag = MPI_Allgather(&local_num_octants,1,MPI_UINT64_T,rbuff,1,MPI_UINT64_T,MPI_COMM_WORLD);
			for(int p = 0; p < nproc; ++p){
				partition_range_globalidx[p] = 0;
				for(int pp = 0; pp <=p; ++pp)
					partition_range_globalidx[p] += rbuff[pp];
				--partition_range_globalidx[p];
			}
			//update partition_range_position
			uint64_t lastDescMorton = octree.getLastDesc().computeMorton();
			error_flag = MPI_Allgather(&lastDescMorton,1,MPI_UINT64_T,partition_last_desc,1,MPI_UINT64_T,MPI_COMM_WORLD);
			uint64_t firstDescMorton = octree.getFirstDesc().computeMorton();
			error_flag = MPI_Allgather(&firstDescMorton,1,MPI_UINT64_T,partition_first_desc,1,MPI_UINT64_T,MPI_COMM_WORLD);
		}
	};

	//=================================================================================//

	void updateAfterCoarse(){							//update Class_Para_Tree members and delete overlapping octants after a coarse
		if(serial){
			updateAdapt();
		}
		else{
			//Only if parallel
			updateAdapt();
			uint64_t lastDescMortonPre, firstDescMortonPost;
			lastDescMortonPre = (rank!=0) * partition_last_desc[rank-1];
			firstDescMortonPost = (rank<nproc-1)*partition_first_desc[rank+1] + (rank==nproc-1)*partition_last_desc[rank];
			octree.checkCoarse(lastDescMortonPre, firstDescMortonPost);
			updateAdapt();
		}
	};

	//=================================================================================//

	void updateAfterCoarse(u32vector & mapidx){			//update Class_Para_Tree members and delete overlapping octants after a coarse
		if(serial){
			updateAdapt();
		}
		else{
			//Only if parallel
			updateAdapt();
			uint64_t lastDescMortonPre, firstDescMortonPost;
			lastDescMortonPre = (rank!=0) * partition_last_desc[rank-1];
			firstDescMortonPost = (rank<nproc-1)*partition_first_desc[rank+1] + (rank==nproc-1)*partition_last_desc[rank];
			octree.checkCoarse(lastDescMortonPre, firstDescMortonPost, mapidx);
			updateAdapt();
		}
	};

	//=================================================================================//
	////TODO Update after coarse with intersections
	//=================================================================================//

	void commMarker(){									// communicates marker of ghosts
		// borderPerProcs has to be built

		//PACK (mpi) LEVEL AND MARKER OF BORDER OCTANTS IN CHAR BUFFERS WITH SIZE (map value) TO BE SENT TO THE RIGHT PROCESS (map key)
		//it visits every element in bordersPerProc (one for every neighbor proc)
		//for every element it visits the border octants it contains and pack its marker in a new structure, sendBuffers
		//this map has an entry Class_Comm_Buffer for every proc containing the size in bytes of the buffer and the octants marker
		//to be sent to that proc packed in a char* buffer
		int8_t marker;
		bool info[16], mod;
		map<int,Class_Comm_Buffer> sendBuffers;
		map<int,vector<uint32_t> >::iterator bitend = bordersPerProc.end();
		uint32_t pbordersOversize = 0;
		for(map<int,vector<uint32_t> >::iterator bit = bordersPerProc.begin(); bit != bitend; ++bit){
			pbordersOversize += bit->second.size();
			//		int buffSize = bit->second.size() * (int)ceil((double)(markerBytes) / (double)(CHAR_BIT/8));
			int buffSize = bit->second.size() * (int)ceil((double)(global3D.markerBytes + global3D.boolBytes) / (double)(CHAR_BIT/8));
			int key = bit->first;
			const vector<uint32_t> & value = bit->second;
			sendBuffers[key] = Class_Comm_Buffer(buffSize,'a');
			int pos = 0;
			int nofBorders = value.size();
			for(int i = 0; i < nofBorders; ++i){
				//the use of auxiliary variable can be avoided passing to MPI_Pack the members of octant but octant in that case cannot be const
				const Class_Octant<3> & octant = octree.octants[value[i]];
				marker = octant.getMarker();
				mod	= octant.info[15];
				error_flag = MPI_Pack(&marker,1,MPI_INT8_T,sendBuffers[key].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
				error_flag = MPI_Pack(&mod,1,MPI::BOOL,sendBuffers[key].commBuffer,buffSize,&pos,MPI_COMM_WORLD);
			}
		}

		//COMMUNICATE THE SIZE OF BUFFER TO THE RECEIVERS
		//the size of every borders buffer is communicated to the right process in order to build the receive buffer
		//and stored in the recvBufferSizePerProc structure
		MPI_Request req[sendBuffers.size()*2];
		MPI_Status stats[sendBuffers.size()*2];
		int nReq = 0;
		map<int,int> recvBufferSizePerProc;
		map<int,Class_Comm_Buffer>::iterator sitend = sendBuffers.end();
		for(map<int,Class_Comm_Buffer>::iterator sit = sendBuffers.begin(); sit != sitend; ++sit){
			recvBufferSizePerProc[sit->first] = 0;
			error_flag = MPI_Irecv(&recvBufferSizePerProc[sit->first],1,MPI_UINT32_T,sit->first,rank,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		map<int,Class_Comm_Buffer>::reverse_iterator rsitend = sendBuffers.rend();
		for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
			error_flag =  MPI_Isend(&rsit->second.commBufferSize,1,MPI_UINT32_T,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		MPI_Waitall(nReq,req,stats);

		//COMMUNICATE THE BUFFERS TO THE RECEIVERS
		//recvBuffers structure is declared and each buffer is initialized to the right size
		//then, sendBuffers are communicated by senders and stored in recvBuffers in the receivers
		//at the same time every process compute the size in bytes of all the level and marker of ghost octants
		uint32_t nofBytesOverProc = 0;
		map<int,Class_Comm_Buffer> recvBuffers;
		map<int,int>::iterator ritend = recvBufferSizePerProc.end();
		for(map<int,int>::iterator rit = recvBufferSizePerProc.begin(); rit != ritend; ++rit){
			recvBuffers[rit->first] = Class_Comm_Buffer(rit->second,'a');
		}
		nReq = 0;
		for(map<int,Class_Comm_Buffer>::iterator sit = sendBuffers.begin(); sit != sitend; ++sit){
			nofBytesOverProc += recvBuffers[sit->first].commBufferSize;
			error_flag = MPI_Irecv(recvBuffers[sit->first].commBuffer,recvBuffers[sit->first].commBufferSize,MPI_PACKED,sit->first,rank,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
			error_flag =  MPI_Isend(rsit->second.commBuffer,rsit->second.commBufferSize,MPI_PACKED,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		MPI_Waitall(nReq,req,stats);

		/*
			//COMPUTE GHOSTS SIZE IN BYTES
			//number of ghosts in every process is obtained through the size in bytes of the single octant
			//and ghost vector in local tree is resized
			uint32_t nofGhosts = nofBytesOverProc / (uint32_t)(levelBytes+markerBytes);
			octree.size_ghosts = nofGhosts;
			octree.ghosts.resize(nofGhosts);
		 */

		//UNPACK BUFFERS AND BUILD GHOSTS CONTAINER OF CLASS_LOCAL_TREE
		//every entry in recvBuffers is visited, each buffers from neighbor processes is unpacked octant by octant.
		//every ghost octant is built and put in the ghost vector
		uint32_t ghostCounter = 0;
		map<int,Class_Comm_Buffer>::iterator rritend = recvBuffers.end();
		for(map<int,Class_Comm_Buffer>::iterator rrit = recvBuffers.begin(); rrit != rritend; ++rrit){
			int pos = 0;
			int nofGhostsPerProc = int(rrit->second.commBufferSize / ((uint32_t) (global3D.markerBytes + global3D.boolBytes)));
			for(int i = 0; i < nofGhostsPerProc; ++i){
				error_flag = MPI_Unpack(rrit->second.commBuffer,rrit->second.commBufferSize,&pos,&marker,1,MPI_INT8_T,MPI_COMM_WORLD);
				octree.ghosts[ghostCounter].setMarker(marker);
				error_flag = MPI_Unpack(rrit->second.commBuffer,rrit->second.commBufferSize,&pos,&mod,1,MPI::BOOL,MPI_COMM_WORLD);
				octree.ghosts[ghostCounter].info[15] = mod;
				++ghostCounter;
			}
		}
		recvBuffers.clear();
		sendBuffers.clear();
		recvBufferSizePerProc.clear();

	};

	//=================================================================================//

	void balance21(){									// 2:1 balancing of parallel octree
		bool globalDone = true, localDone = false;
		int  iteration  = 0;


		writeLog("---------------------------------------------");
		writeLog(" 2:1 BALANCE (balancing Marker before Adapt)");
		writeLog(" ");
		writeLog(" Iterative procedure	");
		writeLog(" ");
		writeLog(" Iteration	:	" + to_string(iteration));


		commMarker();
		localDone = octree.localBalance(true);
		MPI_Barrier(MPI_COMM_WORLD);
		error_flag = MPI_Allreduce(&localDone,&globalDone,1,MPI::BOOL,MPI_LOR,MPI_COMM_WORLD);

		while(globalDone){
			iteration++;
			writeLog(" Iteration	:	" + to_string(iteration));
			commMarker();
			localDone = octree.localBalance(false);
			error_flag = MPI_Allreduce(&localDone,&globalDone,1,MPI::BOOL,MPI_LOR,MPI_COMM_WORLD);
		}

		commMarker();
		writeLog(" Iteration	:	Finalizing ");
		writeLog(" ");
		localDone = octree.localBalance(false);
		commMarker();

		writeLog(" 2:1 Balancing reached ");
		writeLog(" ");
		writeLog("---------------------------------------------");


	};

	//=================================================================================//

	bool adapt(){  										//call refine and coarse on the local tree
		bool globalDone = false, localDone = false;
		uint32_t nocts = octree.getNumOctants();
		vector<Class_Octant<3> >::iterator iter, iterend = octree.octants.end();

		for (iter = octree.octants.begin(); iter != iterend; iter++){
			iter->info[12] = false;
			iter->info[13] = false;
		}
		if(serial){
			writeLog("---------------------------------------------");
			writeLog(" ADAPT (Refine/Coarse)");
			writeLog(" ");

			// 2:1 Balance
			balance21();

			writeLog(" ");
			writeLog(" Initial Number of octants	:	" + to_string(octree.getNumOctants()));

			// Refine
			while(octree.refine());

			if (octree.getNumOctants() > nocts)
				localDone = true;
			writeLog(" Number of octants after Refine	:	" + to_string(octree.getNumOctants()));
			nocts = octree.getNumOctants();
			updateAdapt();
			//		setPboundGhosts();

			// Coarse
			while(octree.coarse());
			if (octree.getNumOctants() < nocts)
				localDone = true;
			writeLog(" Number of octants after Coarse	:	" + to_string(nocts));
			MPI_Barrier(MPI_COMM_WORLD);
			error_flag = MPI_Allreduce(&localDone,&globalDone,1,MPI::BOOL,MPI_LOR,MPI_COMM_WORLD);
			updateAfterCoarse();
			writeLog(" ");
			writeLog("---------------------------------------------");
		}
		else{
			writeLog("---------------------------------------------");
			writeLog(" ADAPT (Refine/Coarse)");
			writeLog(" ");

			// 2:1 Balance
			balance21();

			writeLog(" ");
			writeLog(" Initial Number of octants	:	" + to_string(global_num_octants));
			updateAdapt();			// Togliere se non necessario
			setPboundGhosts();		// Togliere se non necessario

			// Refine
			while(octree.refine());

			if (octree.getNumOctants() > nocts)
				localDone = true;
			updateAdapt();
			setPboundGhosts();
			writeLog(" Number of octants after Refine	:	" + to_string(global_num_octants));
			nocts = octree.getNumOctants();

			// Coarse
			while(octree.coarse());

			if (octree.getNumOctants() < nocts)
				localDone = true;
			MPI_Barrier(MPI_COMM_WORLD);
			error_flag = MPI_Allreduce(&localDone,&globalDone,1,MPI::BOOL,MPI_LOR,MPI_COMM_WORLD);
			updateAfterCoarse();
			setPboundGhosts();
			writeLog(" Number of octants after Coarse	:	" + to_string(global_num_octants));
			writeLog(" ");
			writeLog("---------------------------------------------");
		}
		return globalDone;

	};

	//=================================================================================//

	bool adapt(u32vector & mapidx){  					//call refine and coarse on the local tree
		// mapidx[i] = index in old octants vector of the i-th octant (index of father or first child if octant is new after refine or coarse)
		bool globalDone = false, localDone = false;
		uint32_t nocts = octree.getNumOctants();
		vector<Class_Octant<3> >::iterator iter, iterend = octree.octants.end();

		for (iter = octree.octants.begin(); iter != iterend; iter++){
			iter->info[12] = false;
			iter->info[13] = false;
		}

		// mapidx init
		mapidx.clear();
		mapidx.resize(nocts);
		mapidx.shrink_to_fit();
		for (uint32_t i=0; i<nocts; i++){
			mapidx[i] = i;
		}
		if(serial){
			writeLog("---------------------------------------------");
			writeLog(" ADAPT (Refine/Coarse)");
			writeLog(" ");

			// 2:1 Balance
			balance21();

			writeLog(" ");
			writeLog(" Initial Number of octants	:	" + to_string(octree.getNumOctants()));

			// Refine
			while(octree.refine(mapidx));

			if (octree.getNumOctants() > nocts)
				localDone = true;
			nocts = octree.getNumOctants();
			writeLog(" Number of octants after Refine	:	" + to_string(nocts));

			// Coarse
			while(octree.coarse(mapidx));

			if (octree.getNumOctants() < nocts)
				localDone = true;
			nocts = octree.getNumOctants();
			MPI_Barrier(MPI_COMM_WORLD);
			error_flag = MPI_Allreduce(&localDone,&globalDone,1,MPI::BOOL,MPI_LOR,MPI_COMM_WORLD);
			writeLog(" Number of octants after Coarse	:	" + to_string(nocts));
			updateAfterCoarse(mapidx);
			writeLog(" ");
			writeLog("---------------------------------------------");
		}
		else{
			writeLog("---------------------------------------------");
			writeLog(" ADAPT (Refine/Coarse)");
			writeLog(" ");

			// 2:1 Balance
			balance21();

			writeLog(" ");
			writeLog(" Initial Number of octants	:	" + to_string(global_num_octants));
			updateAdapt();			// Togliere se non necessario
			setPboundGhosts();		// Togliere se non necessario

			// Refine
			while(octree.refine(mapidx));

			if (octree.getNumOctants() > nocts)
				localDone = true;
			nocts = octree.getNumOctants();
			updateAdapt();
			setPboundGhosts();
			writeLog(" Number of octants after Refine	:	" + to_string(global_num_octants));

			// Coarse
			while(octree.coarse(mapidx));

			if (octree.getNumOctants() < nocts)
				localDone = true;
			nocts = octree.getNumOctants();
			MPI_Barrier(MPI_COMM_WORLD);
			error_flag = MPI_Allreduce(&localDone,&globalDone,1,MPI::BOOL,MPI_LOR,MPI_COMM_WORLD);
			updateAfterCoarse(mapidx);
			setPboundGhosts();
			writeLog(" Number of octants after Coarse	:	" + to_string(global_num_octants));
			writeLog(" ");
			writeLog("---------------------------------------------");
		}
		return globalDone;

	};

	//=================================================================================//

	bool adapt(u32vector & mapidx,
			u32vector & mapinters_int,
			u32vector & mapinters_ghost,
			u32vector & mapinters_bord){
		bool globalDone = false, localDone = false;
		uint32_t nocts = octree.getNumOctants();
		vector<Class_Octant<3> >::iterator iter, iterend = octree.octants.end();

		for (iter = octree.octants.begin(); iter != iterend; iter++){
			iter->info[12] = false;
			iter->info[13] = false;
		}

		// mapidx init
		mapidx.clear();
		mapidx.resize(nocts);
		mapidx.shrink_to_fit();
		for (uint32_t i=0; i<nocts; i++){
			mapidx[i] = i;
		}
		if(serial){
			writeLog("---------------------------------------------");
			writeLog(" ADAPT (Refine/Coarse)");
			writeLog(" ");

			// 2:1 Balance
			balance21();

			writeLog(" ");
			writeLog(" Initial Number of octants	:	" + to_string(octree.getNumOctants()));

			// Refine
			while(octree.refine(mapidx));

			if (octree.getNumOctants() > nocts)
				localDone = true;
			nocts = octree.getNumOctants();
			writeLog(" Number of octants after Refine	:	" + to_string(nocts));

			// Coarse
			while(octree.coarse(mapidx));

			if (octree.getNumOctants() < nocts)
				localDone = true;
			nocts = octree.getNumOctants();
			MPI_Barrier(MPI_COMM_WORLD);
			error_flag = MPI_Allreduce(&localDone,&globalDone,1,MPI::BOOL,MPI_LOR,MPI_COMM_WORLD);
			writeLog(" Number of octants after Coarse	:	" + to_string(nocts));
			updateAfterCoarse(mapidx);

			octree.updateIntersections(mapidx,
					mapinters_int,
					mapinters_ghost,
					mapinters_bord);

			writeLog(" ");
			writeLog("---------------------------------------------");
		}
		else{
			writeLog("---------------------------------------------");
			writeLog(" ADAPT (Refine/Coarse)");
			writeLog(" ");

			// 2:1 Balance
			balance21();

			writeLog(" ");
			writeLog(" Initial Number of octants	:	" + to_string(global_num_octants));
			updateAdapt();			// Togliere se non necessario
			setPboundGhosts();		// Togliere se non necessario

			// Refine
			while(octree.refine(mapidx));

			if (octree.getNumOctants() > nocts)
				localDone = true;
			nocts = octree.getNumOctants();
			updateAdapt();
			setPboundGhosts();
			writeLog(" Number of octants after Refine	:	" + to_string(global_num_octants));

			// Coarse
			while(octree.coarse(mapidx));

			if (octree.getNumOctants() < nocts)
				localDone = true;
			nocts = octree.getNumOctants();
			MPI_Barrier(MPI_COMM_WORLD);
			error_flag = MPI_Allreduce(&localDone,&globalDone,1,MPI::BOOL,MPI_LOR,MPI_COMM_WORLD);
			updateAfterCoarse(mapidx);
			setPboundGhosts();

			octree.updateIntersections(mapidx,
					mapinters_int,
					mapinters_ghost,
					mapinters_bord);

			writeLog(" Number of octants after Coarse	:	" + to_string(global_num_octants));
			writeLog(" ");
			writeLog("---------------------------------------------");
		}
		return globalDone;

	};

	//=================================================================================//

	template<class UserDataComm>
	void communicate(UserDataComm & userData){

		//BUILD SEND BUFFERS
		map<int,Class_Comm_Buffer> sendBuffers;
		size_t fixedDataSize = userData.fixedSize();
		map<int,vector<uint32_t> >::iterator bitend = bordersPerProc.end();
		map<int,vector<uint32_t> >::iterator bitbegin = bordersPerProc.begin();
		for(map<int,vector<uint32_t> >::iterator bit = bitbegin; bit != bitend; ++bit){
			const int & key = bit->first;
			const vector<uint32_t> & pborders = bit->second;
			size_t buffSize = 0;
			size_t nofPbordersPerProc = pborders.size();
			if(fixedDataSize != 0){
				buffSize = fixedDataSize*nofPbordersPerProc;
			}
			else{
				for(size_t i = 0; i < nofPbordersPerProc; ++i){
					buffSize += userData.size(pborders[i]);
				}
			}
			//enlarge buffer to store number of pborders from this proc
			buffSize += sizeof(int);
			//build buffer for this proc
			sendBuffers[key] = Class_Comm_Buffer(buffSize,'a');
			//store number of pborders from this proc at the begining
			MPI_Pack(&nofPbordersPerProc,1,MPI_INT,sendBuffers[key].commBuffer,sendBuffers[key].commBufferSize,&sendBuffers[key].pos,MPI_COMM_WORLD);

			//WRITE SEND BUFFERS
			for(size_t j = 0; j < nofPbordersPerProc; ++j){
				userData.gather(sendBuffers[key],pborders[j]);
			}
		}

		//Communicate Buffers Size
		MPI_Request req[sendBuffers.size()*2];
		MPI_Status stats[sendBuffers.size()*2];
		int nReq = 0;
		map<int,int> recvBufferSizePerProc;
		map<int,Class_Comm_Buffer>::iterator sitend = sendBuffers.end();
		for(map<int,Class_Comm_Buffer>::iterator sit = sendBuffers.begin(); sit != sitend; ++sit){
			recvBufferSizePerProc[sit->first] = 0;
			error_flag = MPI_Irecv(&recvBufferSizePerProc[sit->first],1,MPI_UINT32_T,sit->first,rank,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		map<int,Class_Comm_Buffer>::reverse_iterator rsitend = sendBuffers.rend();
		for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
			error_flag =  MPI_Isend(&rsit->second.commBufferSize,1,MPI_UINT32_T,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		MPI_Waitall(nReq,req,stats);

		//Communicate Buffers
		//uint32_t nofBytesOverProc = 0;
		map<int,Class_Comm_Buffer> recvBuffers;
		map<int,int>::iterator ritend = recvBufferSizePerProc.end();
		for(map<int,int>::iterator rit = recvBufferSizePerProc.begin(); rit != ritend; ++rit){
			recvBuffers[rit->first] = Class_Comm_Buffer(rit->second,'a');
		}
		nReq = 0;
		for(map<int,Class_Comm_Buffer>::iterator sit = sendBuffers.begin(); sit != sitend; ++sit){
			//nofBytesOverProc += recvBuffers[sit->first].commBufferSize;
			error_flag = MPI_Irecv(recvBuffers[sit->first].commBuffer,recvBuffers[sit->first].commBufferSize,MPI_PACKED,sit->first,rank,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		for(map<int,Class_Comm_Buffer>::reverse_iterator rsit = sendBuffers.rbegin(); rsit != rsitend; ++rsit){
			error_flag =  MPI_Isend(rsit->second.commBuffer,rsit->second.commBufferSize,MPI_PACKED,rsit->first,rsit->first,MPI_COMM_WORLD,&req[nReq]);
			++nReq;
		}
		MPI_Waitall(nReq,req,stats);

		//READ RECEIVE BUFFERS
		int ghostOffset = 0;
		map<int,Class_Comm_Buffer>::iterator rbitend = recvBuffers.end();
		map<int,Class_Comm_Buffer>::iterator rbitbegin = recvBuffers.begin();
		for(map<int,Class_Comm_Buffer>::iterator rbit = rbitbegin; rbit != rbitend; ++rbit){
			int nofGhostFromThisProc = 0;
			MPI_Unpack(rbit->second.commBuffer,rbit->second.commBufferSize,&rbit->second.pos,&nofGhostFromThisProc,1,MPI_INT,MPI_COMM_WORLD);
			for(int k = 0; k < nofGhostFromThisProc; ++k){
				userData.scatter(rbit->second, k+ghostOffset);
			}
			ghostOffset += nofGhostFromThisProc;
		}


	};

	//=================================================================================//

	void computeConnectivity(){							// Computes nodes vector and connectivity of octants of local tree
		map<uint64_t, vector<double> > mapnodes;
		map<uint64_t, vector<double> >::iterator iter, iterend;
		uint32_t i, k, counter;
		uint64_t morton;
		uint32_t noctants = octree.getNumOctants();
		dvector2D octnodes;
		uint8_t j;

		octnodes.reserve(global3D.nnodes);

		if (nodes.size() == 0){
			connectivity.resize(noctants);
			for (i = 0; i < noctants; i++){
				getNodes(&octree.octants[i], octnodes);
				for (j = 0; j < global3D.nnodes; j++){
					morton = mortonEncode_magicbits(uint32_t(octnodes[j][0]/trans.L*double(global3D.max_length)), uint32_t(octnodes[j][1]/trans.L*double(global3D.max_length)), uint32_t(octnodes[j][2]/trans.L*double(global3D.max_length)));
					if (mapnodes[morton].size()==0){
						mapnodes[morton].reserve(12);
						for (k = 0; k < 3; k++){
							mapnodes[morton].push_back(octnodes[j][k]);
						}
					}
					mapnodes[morton].push_back(double(i));
				}
				dvector2D().swap(octnodes);
			}
			iter	= mapnodes.begin();
			iterend	= mapnodes.end();
			counter = 0;
			uint32_t numnodes = mapnodes.size();
			nodes.resize(numnodes);
			while (iter != iterend){
				vector<double> nodecasting(iter->second.begin(), iter->second.begin()+3);
				//			nodes.push_back(nodecasting);
				nodes[counter] = nodecasting;
				nodes[counter].shrink_to_fit();
				for(vector<double>::iterator iter2 = iter->second.begin()+3; iter2 != iter->second.end(); iter2++){
					if (connectivity[int(*iter2)].size()==0){
						connectivity[int(*iter2)].reserve(8);
					}
					connectivity[int(*iter2)].push_back(counter);
				}
				mapnodes.erase(iter++);
				counter++;
			}
			nodes.shrink_to_fit();
			//Lento. Solo per risparmiare memoria
			for (int ii=0; ii<noctants; ii++){
				connectivity[ii].shrink_to_fit();
			}
			connectivity.shrink_to_fit();
		}
		map<uint64_t, vector<double> >().swap(mapnodes);
		iter = mapnodes.end();

	};
	void clearConnectivity(){							// Clear nodes vector and connectivity of octants of local tree
		dvector2D().swap(nodes);
		u32vector2D().swap(connectivity);
	};
	void updateConnectivity(){		 					// Updates nodes vector and connectivity of octants of local tree
		clearConnectivity();
		computeConnectivity();
	};
	void computeghostsConnectivity(){					// Computes ghosts nodes vector and connectivity of ghosts octants of local tree
		map<uint64_t, vector<double> > mapnodes;
		map<uint64_t, vector<double> >::iterator iter, iterend;
		uint32_t i, k, counter;
		uint64_t morton;
		uint32_t noctants = octree.size_ghosts;
		dvector2D octnodes;
		uint8_t j;

		octnodes.reserve(global3D.nnodes);

		if (ghostsnodes.size() == 0){
			ghostsconnectivity.resize(noctants);
			for (i = 0; i < noctants; i++){
				getNodes(&octree.ghosts[i], octnodes);
				for (j = 0; j < global3D.nnodes; j++){
					morton = mortonEncode_magicbits(uint32_t(octnodes[j][0]/trans.L*double(global3D.max_length)), uint32_t(octnodes[j][1]/trans.L*double(global3D.max_length)), uint32_t(octnodes[j][2]/trans.L*double(global3D.max_length)));
					if (mapnodes[morton].size()==0){
						for (k = 0; k < 3; k++){
							mapnodes[morton].push_back(octnodes[j][k]);
						}
					}
					mapnodes[morton].push_back(i);
				}
				dvector2D().swap(octnodes);
			}
			iter	= mapnodes.begin();
			iterend	= mapnodes.end();
			uint32_t numnodes = mapnodes.size();
			ghostsnodes.resize(numnodes);
			counter = 0;
			while (iter != iterend){
				vector<double> nodecasting(iter->second.begin(), iter->second.begin()+3);
				//			ghostsnodes.push_back(nodecasting);
				ghostsnodes[counter] = nodecasting;
				ghostsnodes[counter].shrink_to_fit();
				for(vector<double>::iterator iter2 = iter->second.begin()+3; iter2 != iter->second.end(); iter2++){
					if (ghostsconnectivity[int(*iter2)].size()==0){
						ghostsconnectivity[int(*iter2)].reserve(8);
					}
					ghostsconnectivity[int(*iter2)].push_back(counter);
				}
				mapnodes.erase(iter++);
				counter++;
			}
			ghostsnodes.shrink_to_fit();
			//Lento. Solo per risparmiare memoria
			for (int ii=0; ii<noctants; ii++){
				ghostsconnectivity[ii].shrink_to_fit();
			}
			ghostsconnectivity.shrink_to_fit();
		}
		iter = mapnodes.end();

	};
	void clearghostsConnectivity(){						// Clear ghosts nodes vector and connectivity of ghosts octants of local tree
		dvector2D().swap(ghostsnodes);
		u32vector2D().swap(ghostsconnectivity);
	};
	void updateghostsConnectivity(){					// Update ghosts nodes vector and connectivity of ghosts octants of local tree
		clearghostsConnectivity();
		computeghostsConnectivity();
	};

	// =================================================================================== //

};

