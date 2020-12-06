#pragma once

#include "euleroperator.h"
#include <glm\vec3.hpp>

HalfEdge* mev(Vertex *v1, glm::vec3 v, Loop*& lp) {
	Vertex *v2 = new Vertex(v);
	HalfEdge* h1 = new HalfEdge(v1, v2, lp), * h2 = new HalfEdge(v2, v1, lp);
	Edge* new_edge = new Edge(h1, h2);
	h1->nextHalfEdge = h2;
	h2->prevHalfEdge = h1;
	h1->hedge = h2->hedge = new_edge;
	HalfEdge* ph = lp->firstHalfEdge;
	if (ph == nullptr) {
		h1->prevHalfEdge = h2;
		h2->nextHalfEdge = h1;
		lp->firstHalfEdge = h1;
	}
	else {
		while (ph->ed !=v1) ph = ph->nextHalfEdge;
		h2->nextHalfEdge = ph->nextHalfEdge;
		ph->nextHalfEdge = h1;
		h1->prevHalfEdge = ph;
		h2->nextHalfEdge->prevHalfEdge = h2;
	}
	Solid* S = lp->lface->fsolid;
	Edge* e = S->firstEdge;
	if (e == nullptr) {
		S->firstEdge = new_edge;
		new_edge->prevEdge = new_edge->nextEdge = new_edge;
	}
	else {
		new_edge->prevEdge = e;
		new_edge->nextEdge = e->nextEdge;
		e->nextEdge = new_edge;
		if (new_edge->nextEdge != nullptr) new_edge->nextEdge->prevEdge = new_edge;
	}
	return h1;
}


Solid* mvsf(glm::vec3 pnt, Vertex *& new_vertex) {
	Solid* new_solid = new Solid();
	Face* new_face = new Face();
	Loop* new_loop = new Loop();
	new_vertex = new Vertex(pnt);
	new_solid->firstFace = new_face;
	new_face->prevFace = new_face->nextFace = new_face;
	new_face->fsolid = new_solid;
	new_face->firstLoop = new_loop;
	new_loop->prevLoop = new_loop->nextLoop = new_loop;
	new_loop->lface = new_face;
	return new_solid;
}


Loop* mef(Vertex *v1, Vertex *v2, Loop*& lp) {//输入需保证原环从v1走向v2
	HalfEdge* h1 = nullptr, * h2 = nullptr;
	HalfEdge* ph = lp->firstHalfEdge;
	while (!h1 || !h2) {
		if (ph->ed == v1) h1 = ph;
		else if (ph->ed == v2) h2 = ph;
		ph = ph->nextHalfEdge;
	}
	HalfEdge* h3 = new HalfEdge(v1, v2), * h4 = new HalfEdge(v2, v1);
	Edge* new_edge = new Edge(h3, h4);
	h3->hedge = h4->hedge = new_edge;
	h4->nextHalfEdge = h1->nextHalfEdge;
	h3->nextHalfEdge = h2->nextHalfEdge;
	h4->nextHalfEdge->prevHalfEdge = h4;
	h3->nextHalfEdge->prevHalfEdge = h3;
	h1->nextHalfEdge = h3;
	h3->prevHalfEdge = h1;
	h2->nextHalfEdge = h4;
	h4->prevHalfEdge = h2;
	lp->firstHalfEdge = h4->nextHalfEdge;
	Loop* new_lp = new Loop(h3);
	
	
	h3->hloop = new_lp;
	//lp->firstHalfEdge = h4;
	h4->hloop = lp;
	//更新半边所属于的loop
	/*for (auto it = lp->firstHalfEdge->nextHalfEdge; it != lp->firstHalfEdge; it = it->nextHalfEdge) {
		it->hloop = lp;
	}*/
	for (auto it = new_lp->firstHalfEdge->nextHalfEdge; it != new_lp->firstHalfEdge; it = it->nextHalfEdge) {
		it->hloop = new_lp;
	}

	Solid* S = lp->lface->fsolid;

	//插入Facelist
	Face* new_face = new Face();
	new_lp->prevLoop = new_lp->nextLoop = new_lp;
	new_lp->lface = new_face;
	new_face->firstLoop = new_lp;
	new_face->fsolid = S;
	Face* f = S->firstFace;
	if (f == nullptr) {
		S->firstFace = new_face;
		new_face->prevFace = new_face->nextFace = new_face;
	}
	else {
		new_face->prevFace = f;
		new_face->nextFace = f->nextFace;
		f->nextFace = new_face;
		if (new_face->nextFace != nullptr) new_face->nextFace->prevFace = new_face;
	}

	//插入到Edgelist
	Edge* e = S->firstEdge;
	if (e == nullptr) {
		S->firstEdge = new_edge;
		new_edge->prevEdge = new_edge->nextEdge = new_edge;
	}
	else {
		new_edge->prevEdge = e;
		new_edge->nextEdge = e->nextEdge;
		e->nextEdge = new_edge;
		if (new_edge->nextEdge != nullptr) new_edge->nextEdge->prevEdge = new_edge;
	}
	return new_lp;
}


Loop* kemr(Vertex *v1, Vertex *v2, Loop* &lp) { //保证v1属于外环
	HalfEdge* ph = lp->firstHalfEdge;
	HalfEdge* h1 = nullptr, * h2 = nullptr;
	while (!h1 || !h2) {
		if (ph->st ==v1 && ph->ed == v2) h1 = ph;
		else if (ph->st == v2 && ph->ed ==v1) h2 = ph;
		ph = ph->nextHalfEdge;
	}
	h1->prevHalfEdge->nextHalfEdge = h2->nextHalfEdge;
	h2->prevHalfEdge->nextHalfEdge = h1->nextHalfEdge;
	Loop* new_loop = new Loop(h1->nextHalfEdge);
	ph = h1->nextHalfEdge;
	while (ph != new_loop->firstHalfEdge) {
		//更新内环所有半边所属于的Loop
		ph->hloop = new_loop;
	}
	//插入到Looplist
	new_loop->lface = lp->lface;
	new_loop->prevLoop = lp;
	new_loop->nextLoop = lp->nextLoop;
	lp->nextLoop = new_loop;
	if (new_loop->nextLoop != nullptr) new_loop->nextLoop->prevLoop = new_loop;
	
	//在edgelist删除原边
	Edge *delete_edge = h1->hedge;
	if (delete_edge->prevEdge == nullptr) {
		delete_edge->nextEdge->prevEdge = nullptr;
		//todo 如果delete_edge->nextEdge == nullptr会报错 
	}
	else {
		delete_edge->prevEdge->nextEdge = delete_edge->nextEdge;
		if (delete_edge->nextEdge) delete_edge->nextEdge->prevEdge = delete_edge->prevEdge;
	}
/*
    //todo 太麻烦了, 输入时自己保证吧
	Solid* S = h1->hloop->lface->fsolid;
	if (S->firstEdge == delete_edge) {
	}
*/
	return new_loop;
}


void kfmrh(Face *f, Face *delete_face) {
	Solid *S = f->fsolid;
	Face* ps = S->firstFace;

	//在facelist中删除
	if (ps == delete_face) S->firstFace = f;
	if (delete_face->prevFace == nullptr) {
		delete_face->nextFace->prevFace = nullptr;
		//todo 如果delete_face->nextEdge == nullptr会报错 
	}
	else {
		delete_face->prevFace->nextFace = delete_face->nextFace;
		if (delete_face->nextFace) delete_face->nextFace->prevFace = delete_face->prevFace;
	}

	//在f中添加delete_face的loop
	auto delete_face_lp = delete_face->firstLoop;
	auto lp = f->firstLoop;
	do {
		//添加loop操作
		auto new_lp = delete_face_lp;
		delete_face_lp = delete_face_lp->nextLoop;
		new_lp->lface = f;
	    new_lp->prevLoop = lp;
	    new_lp->nextLoop = lp->nextLoop;
	    lp->nextLoop = new_lp;
	    if (new_lp->nextLoop != nullptr) new_lp->nextLoop->prevLoop = new_lp;
	} while (delete_face_lp != delete_face->firstLoop);
	//delete delete_face;
}


void sweep(Face* f, glm::vec3 dir, double d) {
	for (int i = 0; i < 3; i++) dir[i] *= d;
	auto lp = f->firstLoop;
	do {
		HalfEdge* ph = lp->firstHalfEdge;
		Vertex *firstv = ph->st;
		ph = ph->nextHalfEdge;
		glm::vec3 firstup_pnt(firstv->pnt[0] + dir[0], firstv->pnt[1] +dir[1],firstv->pnt[2] +dir[2]);/*firstv + dir;*/
		Vertex *firstup = mev(firstv, firstup_pnt, lp)->ed;
		Vertex *prevup = firstup;
		Vertex *nextv = ph->st;
		while (nextv != firstv) {
			glm::vec3 up_pnt(nextv->pnt[0] + dir[0], nextv->pnt[1] + dir[1],nextv->pnt[2] + dir[2]);/*nextv + dir;*/
			Vertex *up = mev(nextv, up_pnt, lp)->ed;
			mef(up, prevup, lp);
			prevup = up;
			ph = ph->nextHalfEdge;
			nextv = ph->st;
		}
		mef(firstup, prevup, lp);
		lp = lp->nextLoop;
	} while (lp != f->firstLoop);
}