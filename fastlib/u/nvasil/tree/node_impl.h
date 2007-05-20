#ifndef NODE_IMPL_H_
#define NODE_IMPL_H_

#define TEMPLATE__                                               \
template<typename TYPELIST, bool diagnostic>                                                                  

#define NODE__                                                   \
	Node<TYPELIST, diagnostic>                  
 
TEMPLATE__    
NODE__::Node() {
  left_.SetNULL();
  right_.SetNULL();
  points_.SetNULL();
  kneighbors_=NULL;
	node_id_ = numeric_limits<index_t>::max();
	min_dist_so_far_=numeric_limits<Precision_t>::max();
}

TEMPLATE__
void NODE__::Init(const BoundingBox_t &box, 
	                const NodeCachedStatistics_t &statistics,		
			            index_t node_id,
			            index_t num_of_points) {
  box_.Alias(box);
  statistics_.Alias(statistics);
  node_id_ = node_id;
	num_of_points_ = num_of_points;
}

TEMPLATE__
void NODE__::Init(const typename NODE__::BoundingBox_t &box,
		      	      const typename NODE__::NodeCachedStatistics_t &statistics,
			            index_t node_id,
                  index_t start,
                  index_t num_of_points,
							    int32 dimension,
			            BinaryDataset<Precision_t> *dataset) {
	box_.Alias(box);
	statistics_.Alias(statistics);
	node_id_ = node_id;
	num_of_points_ = num_of_points;
	points_.Reset(Allocator_t::template malloc<Precision_t>
			             (num_of_points_*dimension));
  index_.Reset(Allocator_t::template malloc<index_t>(num_of_points_));
	for(index_t i=start; i<start+num_of_points_; i++) {
	  for(int32 j=0; j<dimension; j++) {
		  points_[(i-start)*dimension+j]=dataset->At(i,j);
	  }
		index_[i-start]=dataset->get_id(i);
	}
} 

TEMPLATE__
NODE__::~Node() {
}

TEMPLATE__
void *NODE__::operator new(size_t size) {
  typename Allocator_t::template Ptr<Node_t> temp;
	temp.Reset(Allocator_t::malloc(size));
  return (void *)temp.get();
}
     	
TEMPLATE__
void NODE__::operator delete(void *p) {
}

TEMPLATE__
void NODE__::InitKNeighbors(int32 knns) {
	for(index_t i=0; i<num_of_points_; i++) {
	  for(int32 j=0; j<knns; j++) {
		  kneighbors_[i*knns+j].point_id_=index_[i];
		}
	} 
}
                 	
TEMPLATE__
template<typename POINTTYPE>
pair<typename NODE__::NodePtr_t, typename NODE__::NodePtr_t>                     
NODE__::ClosestChild(POINTTYPE point, int32 dimension, 
		                 ComputationsCounter<diagnostic> &comp) {
  return box_.ClosestChild(left_, right_, point, dimension, comp);
}

TEMPLATE__
inline 
pair<pair<typename NODE__::NodePtr_t, typename NODE__::Precision_t>, 
		 pair<typename NODE__::NodePtr_t, typename NODE__::Precision_t> > 
NODE__::ClosestNode(typename NODE__::NodePtr_t ptr1,
		                typename NODE__::NodePtr_t ptr2,
									  int32 dimension,
							      ComputationsCounter<diagnostic> &comp) {
	Precision_t dist1 = BoundingBox_t::Distance(box_, ptr1->get_box(), 
			                                      dimension, comp);
	Precision_t dist2 = BoundingBox_t::Distance(box_, ptr2->get_box(), 
			                                      dimension, comp);
  if (dist1<dist2) {
	  return make_pair(make_pair(ptr1, dist1), make_pair(ptr2, dist2));
	} else {
	  return make_pair(make_pair(ptr2,dist2), make_pair(ptr1, dist1));
	}
}

TEMPLATE__
template<typename POINTTYPE, typename NEIGHBORTYPE>
inline void NODE__::FindNearest(POINTTYPE query_point, 
    vector<pair<typename NODE__::Precision_t, 
		            typename NODE__::Point_t> > &nearest, 
		NEIGHBORTYPE range, 
		int32 dimension,
		typename NODE__::PointIdDiscriminator_t &discriminator,
    ComputationsCounter<diagnostic> &comp) {
  
	for(index_t i=0; i<num_of_points_; i++) {
  	comp.UpdateDistances();
  	//  we have to check if we are comparing the point with itself
 	  if (unlikely(discriminator.AreTheSame(index_[i], 
		       	     query_point.get_id())==true)) {
  	 	continue;
 	  } 

		Precision_t dist = BoundingBox_t::
			template Distance(query_point, 
			                  points_.get()+i*dimension,
			  							  dimension);
		// In case it is range nearest neighbors
		if (Loki::TypeTraits<NEIGHBORTYPE>::isStdFloat==true) {
		  if (dist<=range){
				Point_t point;
				point.Alias(points_.get()+i*dimension, index_[i]);
			  nearest.push_back(make_pair(dist, point));
			}
		} else {
			// for k nearest neighbors
		  Point_t point;
			point.Alias(points_.get()+i*dimension, index_[i]);
			nearest.push_back(make_pair(dist, point));
		}
	}
	
  // for k-nearest neighbors 
  if (Loki::TypeTraits<NEIGHBORTYPE>::isStdFloat==false) {
	  typename  std::vector<pair<Precision_t, Point_t> >::iterator it;
		//it=nearest.begin()+(index_t)range;
		index_t j=0;
		for(it=nearest.begin(), j=0; j<(index_t)range; j++) {
		  it++;
		}
  	std::sort(nearest.begin(), 
				      nearest.end(),
							PairComparator());
		if (likely(nearest.size()>(uint32)range)) {
		  nearest.erase(it, nearest.end());
		} else {
			pair<Precision_t, Point_t> dummy;
			dummy.first=numeric_limits<Precision_t>::max();
			index_t extra_size=(index_t)(range-nearest.size());
		  for(index_t i=0; i<extra_size; i++) {
			  nearest.push_back(dummy);
			}
		}	  
	}
}

TEMPLATE__
template<typename NEIGHBORTYPE>
inline void NODE__::FindAllNearest(
		                NodePtr_t query_node,
                    typename NODE__::Precision_t &max_neighbor_distance,
                    NEIGHBORTYPE range,
                    int32 dimension,
										typename NODE__::PointIdDiscriminator_t &discriminator,
                    ComputationsCounter<diagnostic> &comp) {
  Precision_t max_local_distance = 0;
	for(index_t i=0; i<query_node->num_of_points_; i++) {
		Precision_t distance;
		// for k nearest neighbors
		if (Loki::TypeTraits<NEIGHBORTYPE>::isStdFloat==false) {
     	// get the current maximum distance for the specific point
  	  distance = query_node->kneighbors_[i*(int32)range+(int32)range-1].distance_;
		} else {
		  distance=range;
		}
    		  // for k nearest neighbors
    if (Loki::TypeTraits<NEIGHBORTYPE>::isStdFloat==false) {                                          	
		  vector<pair<Precision_t, Point_t> > temp((index_t)range);
			for(int32 j=0; j<range; j++) {
			  temp[j].first=query_node->kneighbors_[i*(index_t)range+j].distance_;
				temp[j].second=query_node->kneighbors_[i*(index_t)range+j].nearest_;
			}
			Point_t point;
			point.Alias(query_node->points_.get()+i*dimension, 
				          query_node->index_[i]);
      FindNearest(point, temp, 
                  range, dimension,
			 				    discriminator,	comp);
			DEBUG_ASSERT_MSG((index_t)temp.size()==range, 
			    "During  %i-nn seach, returned %u results",(int)range, 
					(unsigned int)temp.size());

				
			for(int32 j=0; j<(index_t)range; j++) {
			  query_node->kneighbors_[i*(index_t)range+j].distance_=temp[j].first;
			  query_node->kneighbors_[i*(index_t)range+j].nearest_=temp[j].second;
			}
      // Estimate the  maximum nearest neighbor distance
      comp.UpdateComparisons();
      if  (max_local_distance < temp.back().first) {
        max_local_distance = temp.back().first;
      }  
		}	else {
		  // for range nearest neighbors
	    vector<pair<Precision_t, Point_t> >  temp;
			temp.clear();
			Point_t point;
			point.Alias(query_node->points_.get()+i*dimension, 
					        query_node->index_[i]);
		  FindNearest(point, temp, 
                  range, dimension,
			  				  discriminator,	comp);
			for(index_t j=0; j<(index_t)temp.size(); j++) {
			  NNResult result;
				result.point_id_=query_node->index_[i];
				result.nearest_.Alias(temp[j].second);
				result.distance_=temp[j].first;
				if (fwrite(&result, sizeof(NNResult), 1, range_nn_fp_)!=1) {
				  FATAL("Error while writing range nearest neighbors: %s\n",
					      strerror(errno));
				}
			}
		}				  
	}
	if (Loki::TypeTraits<NEIGHBORTYPE>::isStdFloat==true) {
	  max_neighbor_distance=range;
	} else {
	  if (max_neighbor_distance>max_local_distance) {
		  max_neighbor_distance=max_local_distance;
		}
	}
}

TEMPLATE__
string NODE__::Print(int32 dimension) {
  char buf[8192];
  string str;
  if (!IsLeaf()) {
    sprintf(buf, "Node: %llu\n", (unsigned long long)node_id_);
    str.append(buf);
  } else {
  	sprintf(buf, "Leaf: %llu\n", (unsigned long long)node_id_);
  	str.append(buf);
  }
  str.append(box_.Print(dimension));  
	str.append("num_of_points: ");
  sprintf(buf,"%llu\n", (unsigned long long)num_of_points_);
  str.append(buf);
	if (IsLeaf()) {
  	for(index_t i=0; i<num_of_points_; i++) {
  	  for(int32 j=0; j<dimension; j++) {
  	  	sprintf(buf,"%lg ", points_[i*dimension+j]);
  	  	str.append(buf);
  	  }
  	  sprintf(buf, "-%llu \n",(unsigned long long) index_[i]);
  	  str.append(buf); 
  	}
  }
  return str;
}    	 

#undef TEMPLATE__
#undef NODE__     	               	
#endif /*NODE_IMPL_H_*/
