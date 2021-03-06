#pragma once

#include "Math/Vec2.h"
#include <vector>
using namespace std;

#include <SFML/System.hpp>

// Axis-aligned Bounding Box
struct AABB2D {
	// Center of the region
	Vec2 m_pos;
	// Vector to one of its corners (half the diagonal)
	Vec2 m_half;

	AABB2D(){};

	AABB2D(Vec2 p_pos, Vec2 p_half){
		m_pos = p_pos;
		m_half = p_half;
	}

	bool Contains(const Vec2 p_point) const{
		Vec2 l_min_point = m_pos - m_half;
		if(p_point.x >= l_min_point.x && p_point.y >= l_min_point.y){
			Vec2 l_max_point = m_pos + m_half;
			return p_point.x <= l_max_point.x && p_point.y <= l_max_point.y;
		}
		return false;
	}

	bool Intersects(const AABB2D & p_sec) const{
		double l_diff_x = abs(m_pos.x - p_sec.m_pos.x);
		double l_diff_y = abs(m_pos.y - p_sec.m_pos.y);

		if (l_diff_x > (m_half.x + p_sec.m_half.x) || l_diff_y > (m_half.y + p_sec.m_half.y)){
			return false;
		}

		return true;
	}
};

template<class T>
class QuadTree {
public:

	QuadTree(void){
		m_northWest = NULL;
		m_northEast = NULL;
		m_southWest = NULL;
		m_southEast = NULL;
		m_divided = false;
		m_branch_depth = 0;
	}

	~QuadTree(void) {
		if(m_divided){
			delete m_northWest;
			delete m_northEast;
			delete m_southEast;
			delete m_southWest;
		}
	}

	QuadTree(AABB2D p_boundary, int p_depth) {
		m_boundary = p_boundary;
		m_divided = false;
		m_branch_depth = p_depth;
		m_elements_branch = 0;
	}

	bool Insert(const T p_element, Vec2 p_pos){
		// Exit if the element doesn't belong here
		if(!m_boundary.Contains(p_pos)){
			return false;
		}

		if (!m_divided) {	// Base case

			// Insert the element if there is space in this leaf
			if(m_elements.size() < 4){
				m_elements.push_back(make_pair(p_element, p_pos));
				return true;
			}

			// If the element doesn't fit, subdivide the leaf
			Subdivide();
		}

		// Try to insert the element in any of its leaves
		if(m_northWest->Insert(p_element, p_pos) || m_northEast->Insert(p_element, p_pos) 
			|| m_southEast->Insert(p_element, p_pos) || m_southWest->Insert(p_element, p_pos)){
				return true;
		}

		// If we get here, something weird happened
		return false;
	}

	bool Insert(const T p_element, AABB2D p_range){
		if(!m_boundary.Intersects(p_range))
			return false;

		if(!m_divided){
			if(m_elements.size() < 4){
				m_elements.push_back(make_pair(p_element, p_range));
				return true;
			}

			Subdivide();
		}

		if(m_northWest->m_boundary.Intersects(p_range)){
			m_northWest->Insert(p_element, p_range);
		}
		if(m_northEast->m_boundary.Intersects(p_range)){
			m_northEast->Insert(p_element, p_range);
		}
		if(m_southEast->m_boundary.Intersects(p_range)){
			m_southEast->Insert(p_element, p_range);
		}
		if(m_southWest->m_boundary.Intersects(p_range)){
			m_southWest->Insert(p_element, p_range);
		}

		return true;
	}

	bool Insert2(const T p_element, AABB2D p_range){
		if(!m_boundary.Intersects(p_range))
			return false;

		m_elements_branch++;

		if(m_branch_depth == C_MAX_TREE_DEPTH){
			m_elements.push_back(p_element);
			m_elements_regions.push_back(p_range);
			return true;
		}

		if(!m_divided){
			Subdivide2();	
		}

		if(m_northWest->m_boundary.Intersects(p_range)){
			m_northWest->Insert2(p_element, p_range);
		}
		if(m_northEast->m_boundary.Intersects(p_range)){
			m_northEast->Insert2(p_element, p_range);
		}
		if(m_southEast->m_boundary.Intersects(p_range)){
			m_southEast->Insert2(p_element, p_range);
		}
		if(m_southWest->m_boundary.Intersects(p_range)){
			m_southWest->Insert2(p_element, p_range);
		}

		return true;
	}

	vector<T> QueryRange(Vec2 p_pos) {
		//sf::Clock timer;
		QuadTree * l_current_leaf = this;

		while (l_current_leaf->m_divided) {
			if(l_current_leaf->m_northWest->m_boundary.Contains(p_pos)){
				l_current_leaf = l_current_leaf->m_northWest;
			} else if (l_current_leaf->m_northEast->m_boundary.Contains(p_pos)){
				l_current_leaf = l_current_leaf->m_northEast;
			} else if (l_current_leaf->m_southEast->m_boundary.Contains(p_pos)){
				l_current_leaf = l_current_leaf->m_southEast;
			} else if (l_current_leaf->m_southWest->m_boundary.Contains(p_pos)){
				l_current_leaf = l_current_leaf->m_southWest;
			} else {
				return vector<T>();
			}
		}
		//std::cout << timer.getElapsedTime().asMicroseconds() << std::endl;
		//timer.restart();
		vector<T> r_elements;
		for (int i = 0; i < l_current_leaf->m_elements.size(); i++){
			if(l_current_leaf->m_elements_regions[i].Contains(p_pos)){
				r_elements.push_back(l_current_leaf->m_elements[i]);
			}
		}
		//std::cout << timer.getElapsedTime().asMicroseconds() << std::endl;

		return r_elements;
	}

	static void SetMaxDepth(int i){
		if (i > 0)
			C_MAX_TREE_DEPTH = i;
	}

	AABB2D m_boundary;
private:
	void Subdivide() {
		m_divided = true;

		Vec2 l_new_half = m_boundary.m_half / 2;

		Vec2 l_nw_pos = m_boundary.m_pos - l_new_half;
		AABB2D l_northWest(l_nw_pos, l_new_half);
		m_northWest = new QuadTree<T>(l_northWest, m_branch_depth + 1);

		Vec2 l_ne_pos(l_nw_pos.x + m_boundary.m_half.x, l_nw_pos.y);
		AABB2D l_nothEast(l_ne_pos, l_new_half);
		m_northEast = new QuadTree<T>(l_nothEast, m_branch_depth + 1);

		Vec2 l_se_pos = m_boundary.m_pos + l_new_half;
		AABB2D l_southEast(l_se_pos, l_new_half);
		m_southEast = new QuadTree<T>(l_southEast, m_branch_depth + 1);

		Vec2 l_sw_pos(l_nw_pos.x, l_nw_pos.y + m_boundary.m_half.y);
		AABB2D l_southWest(l_sw_pos, l_new_half);
		m_southWest = new QuadTree<T>(l_southWest, m_branch_depth + 1);

		//vector<pair<T, AABB2D>>::iterator iter;
		for (auto const& element: m_elements){
			if(m_northWest->m_boundary.Intersects(element->second)){
				m_northWest->Insert(element->first, element->second);
			}
			if(m_northEast->m_boundary.Intersects(element->second)){
				m_northEast->Insert(element->first, element->second);
			}
			if(m_southEast->m_boundary.Intersects(element->second)){
				m_southEast->Insert(element->first, element->second);
			}
			if(m_southWest->m_boundary.Intersects(element->second)){
				m_southWest->Insert(element->first, element->second);
			}
		}
		m_elements.clear();
	}

	static int C_MAX_TREE_DEPTH;

	void Subdivide2() {
		m_divided = true;

		Vec2 l_new_half = m_boundary.m_half / 2;

		Vec2 l_nw_pos = m_boundary.m_pos - l_new_half;
		AABB2D l_northWest(l_nw_pos, l_new_half);
		m_northWest = new QuadTree<T>(l_northWest, m_branch_depth + 1);

		Vec2 l_ne_pos(l_nw_pos.x + m_boundary.m_half.x, l_nw_pos.y);
		AABB2D l_nothEast(l_ne_pos, l_new_half);
		m_northEast = new QuadTree<T>(l_nothEast, m_branch_depth + 1);

		Vec2 l_se_pos = m_boundary.m_pos + l_new_half;
		AABB2D l_southEast(l_se_pos, l_new_half);
		m_southEast = new QuadTree<T>(l_southEast, m_branch_depth + 1);

		Vec2 l_sw_pos(l_nw_pos.x, l_nw_pos.y + m_boundary.m_half.y);
		AABB2D l_southWest(l_sw_pos, l_new_half);
		m_southWest = new QuadTree<T>(l_southWest, m_branch_depth + 1);
	}

	vector<T> m_elements;
	vector<AABB2D> m_elements_regions;

	QuadTree *m_northWest;
	QuadTree *m_northEast;
	QuadTree *m_southEast;
	QuadTree *m_southWest;

	bool m_divided;
	int m_branch_depth;
	int m_elements_branch;
};

template <class T>
int QuadTree<T>::C_MAX_TREE_DEPTH = 6;
