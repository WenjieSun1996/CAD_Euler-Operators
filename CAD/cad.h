#pragma once

#include <gl/glut.h>
#include <glm\vec3.hpp>

class Solid;
class Loop;
class HalfEdge;
class Vertex;
class Edge;
class HalfEdge;
class Face;

class Solid {
public:
	Solid() {}
	Solid* prevSolid, * nextSolid;
	Face* firstFace;
	Edge* firstEdge;
};


class Face {
public:
	Face() {}
	Face* prevFace, * nextFace;
	Loop* firstLoop;
	Solid* fsolid;
};


class Loop {
public:
	Loop() {}
	Loop(HalfEdge* e) : firstHalfEdge(e) {}
	Loop* prevLoop, *nextLoop;
	HalfEdge* firstHalfEdge;
	Face* lface;
};


class Vertex {
public:
	Vertex() {}
	Vertex(glm::vec3 p) :pnt(p) {	
		points[0] = p[0];
		points[1] = p[1];
		points[2] = p[2];
	};
	GLdouble points[3];
	glm::vec3 pnt;
};


class HalfEdge {
public:
	HalfEdge() {}
	HalfEdge(Vertex *v1 = nullptr, Vertex *v2 = nullptr, Loop* lp = nullptr) : st(v1), ed(v2), hloop(lp) {}
	HalfEdge* prevHalfEdge, * nextHalfEdge;
	Loop* hloop;
	Vertex* st, * ed;
	Edge* hedge;
};


class Edge {
public:
	Edge() {}
	Edge(HalfEdge* l = nullptr, HalfEdge* r = nullptr) :l_halfedge(l), r_halfedge(r) {}
	Edge* prevEdge, * nextEdge;
	HalfEdge* l_halfedge, *r_halfedge;
};