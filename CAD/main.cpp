#include <gl/glew.h>
#include <gl/glut.h>
#include <GLFW/glfw3.h>
#include "euleroperator.h"
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include "cad.h"

bool isLine = false;

class Controller {
public:
	static bool Initialize() {
		return true;
	}

	static void Release() {
	}

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_N && action == GLFW_PRESS) {
			count++;
		}
		if (key >= 0 && key < 1024) {
			if (action == GLFW_PRESS)
				keys[key] = true;
			else if (action == GLFW_RELEASE)
				keys[key] = false;
		}
		if (key == GLFW_KEY_L) {
			std::cout << "Line Mode\n";
			isLine = true;
		}
		if (key == GLFW_KEY_K){
			std::cout << "Solid Mode\n";
			isLine = false;
		}
	}

	static void Movement(float deltaTime) {
		deltaTime *= 50;
		if (keys[GLFW_KEY_W])
			yAngle += 1.0f * deltaTime;
		if (keys[GLFW_KEY_S])
			yAngle -= 1.0f * deltaTime;
		if (keys[GLFW_KEY_A])
			zAngle += 1.0f * deltaTime;
		if (keys[GLFW_KEY_D])
			zAngle -= 1.0f * deltaTime;
	}

private:
	static bool keys[1024];

public:
	static float yAngle, zAngle;
	static unsigned int count;
};

float Controller::yAngle = 0.0;
float Controller::zAngle = 0.0;
unsigned int Controller::count = 0;
bool Controller::keys[1024] = {false};


const glm::ivec2 SCREEN_SIZE(1280, 640);
const float FAR_PLANE = 100.0f;
const float NEAR_PLANE = 1.0f;

glm::mat4 view;
glm::vec3 viewPosition;
glm::vec3 viewDirection;

GLFWwindow* window;


void CALLBACK tessBeginCB(GLenum which) {
	glBegin(which);
}


void CALLBACK tessEndCB() {
	glEnd();
}


void CALLBACK tessVertexCB(const GLvoid* data) {
	const GLdouble* ptr = (const GLdouble*)data;
	glColor3dv(ptr + rand()%5);
	//glColor3d(ptr[0]+0.3, ptr[1]+0.3, ptr[2]+0.3);
	//glColor3dv(ptr+3);
	glVertex3dv(ptr);
}


void DrawFace(Face* face) {
	GLUtesselator* tobj = gluNewTess();
	gluTessCallback(tobj, GLU_TESS_BEGIN, (void(CALLBACK*)())tessBeginCB);
	gluTessCallback(tobj, GLU_TESS_END, (void(CALLBACK*)())tessEndCB);
	gluTessCallback(tobj, GLU_TESS_VERTEX, (void(CALLBACK*)())tessVertexCB);
	gluTessBeginPolygon(tobj, nullptr);
	Loop* lp = face->firstLoop;
	do {
		gluTessBeginContour(tobj);
		HalfEdge* edge = lp->firstHalfEdge;
		do {
			//std::cout << edge->st->points[0] << " " << edge->st->points[1] <<" " << edge->st->points[2] << std::endl;
			gluTessVertex(tobj, edge->st->points, edge->st->points);
			edge = edge->nextHalfEdge;
		} while (edge != lp->firstHalfEdge);
		lp = lp->nextLoop;
		gluTessEndContour(tobj);
	} while (lp != face->firstLoop);
	gluTessEndPolygon(tobj);
	gluDeleteTess(tobj);
}


void DrawLine(Face* face) {
	glColor3f(1, 0, 0);
	glLineWidth(3);
	Loop* lp = face->firstLoop;
	do {
		HalfEdge* edge = lp->firstHalfEdge;
		do {
			glBegin(GL_LINES);
			    glVertex3f(edge->st->pnt.x, edge->st->pnt.y, edge->st->pnt.z);
				glVertex3f(edge->ed->pnt.x, edge->ed->pnt.y, edge->ed->pnt.z);
			glEnd();
			edge = edge->nextHalfEdge;
		} while (edge != lp->firstHalfEdge);
		lp = lp->nextLoop;
	} while (lp != face->firstLoop);
}


void DrawSolid(Solid *S) {
	auto pf = S->firstFace;
	do {
		if (!isLine) DrawFace(pf);
		else DrawLine(pf);
		pf = pf->nextFace;
	} while (pf != S->firstFace);
	glFlush();
}


Solid* Read() {
	std::fstream fin;
	Solid* S = nullptr;
	Vertex* pnt_0 = nullptr;
	Loop* lp_0 = nullptr , *lp_1 = nullptr;
	fin.open("data\\data.txt");
	int NumOfLoop, NumOfPoint;
	fin >> NumOfLoop;
	for (int i = 0; i < NumOfLoop; ++i) {
		fin >> NumOfPoint;
		if (i == 0) {                  //外环
			Vertex* pre_pnt = nullptr;
			for (int j = 0; j < NumOfPoint; ++j) {
				glm::vec3 v;
				fin >> v.x >> v.y >> v.z;
				if (j == 0) {          //初始点
					S = mvsf(v, pnt_0);
					lp_0 = S->firstFace->firstLoop;
					pre_pnt = pnt_0;
				}
				else {                 //依次读入其他点
					pre_pnt = mev(pre_pnt, v, lp_0)->ed;
				}
			}
			lp_1 = mef(pnt_0, pre_pnt, lp_0);
		}
		else {                       //内环
			Vertex* pre_pnt = pnt_0, *first_pnt_of_inner_loop = nullptr;
			for (int j = 0; j < NumOfPoint; ++j) {
				glm::vec3 v;
				fin >> v.x >> v.y >> v.z;
				pre_pnt = mev(pre_pnt, v, lp_1)->ed;
				if (j == 0) first_pnt_of_inner_loop = pre_pnt;
			}
			auto lp_2 = kemr(pnt_0, first_pnt_of_inner_loop, lp_1);
			auto lp_3 = mef(first_pnt_of_inner_loop, pre_pnt, lp_2);
			kfmrh(lp_0->lface, lp_3->lface);
		}
	}
	glm::vec3 dir;
	int dist;
	fin >> dir.x >> dir.y >> dir.z >> dist;;
	sweep(S->firstFace, dir, dist);
	return S;
	/*
	Vertex *pnt_0;
	glm::vec3 v(2, 2, 0 );
	Solid* S = mvsf(v, pnt_0);
	Loop* lp_0 = S->firstFace->firstLoop;
	auto edge_0 = mev(pnt_0, glm::vec3(6, 2, 0), lp_0);
	auto pnt_1 = edge_0->ed;
	auto edge_1 = mev(pnt_1, glm::vec3(6, 6, 0), lp_0);
	auto pnt_2 = edge_1->ed;
	auto edge_2 = mev(pnt_2, glm::vec3(2, 6, 0), lp_0);
	auto pnt_3 = edge_2->ed;
	auto lp_1 = mef(pnt_0, pnt_3, lp_0);

	auto edge_3 = mev(pnt_0, glm::vec3(3, 3, 0), lp_1);
	auto pnt_4 = edge_3->ed;
	auto edge_4 = mev(pnt_4, glm::vec3(5, 3,0 ), lp_1);
	auto pnt_5 = edge_4->ed;
	auto edge_5 = mev(pnt_5, glm::vec3(5, 5 ,0 ), lp_1);
	auto pnt_6 = edge_5->ed;
	auto edge_6 = mev(pnt_6, glm::vec3(3, 5, 0), lp_1);
	auto pnt_7 = edge_6->ed;

	auto lp_2 = kemr(pnt_0, pnt_4, lp_1);
	auto lp_3 = mef(pnt_4, pnt_7, lp_2);
	kfmrh(lp_0->lface, lp_3->lface);

	sweep(S->firstFace, glm::vec3(0, 0, 1), 1.50);
	return S;
	*/

	/*
	Vertex* pnt_0;
	glm::vec3 v(1.52, 5.92, 0);
	Solid* S = mvsf(v, pnt_0);
	Loop* lp_0 = S->firstFace->firstLoop;
	auto edge_0 = mev(pnt_0, glm::vec3(3.43, 5.92, 0), lp_0);
	auto pnt_1 = edge_0->ed;
	auto edge_1 = mev(pnt_1, glm::vec3(2.50, 4.54, 0), lp_0);
	auto pnt_2 = edge_1->ed;
	auto edge_2 = mev(pnt_2, glm::vec3(2.81, 4.17, 0), lp_0);
	auto pnt_3 = edge_2->ed;
	auto edge_3 = mev(pnt_3, glm::vec3(4.35, 5.91, 0), lp_0);
	auto pnt_4 = edge_3->ed;
	auto edge_4 = mev(pnt_4, glm::vec3(4.69, 5.91, 0), lp_0);
	auto pnt_5 = edge_4->ed;
	auto edge_5 = mev(pnt_5, glm::vec3(4.69, 4.16, 0), lp_0);
	auto pnt_6 = edge_5->ed;
	auto edge_6 = mev(pnt_6, glm::vec3(5.86, 2.89, 0), lp_0);
	auto pnt_7 = edge_6->ed;
	auto edge_7 = mev(pnt_7, glm::vec3(5.63, 2.68, 0), lp_0);
	auto pnt_8 = edge_7->ed;
	auto edge_8 = mev(pnt_8, glm::vec3(4.69, 3.71, 0), lp_0);
	auto pnt_9 = edge_8->ed;
	auto edge_9 = mev(pnt_9, glm::vec3(4.69, 1.61, 0), lp_0);
	auto pnt_10 = edge_9->ed;
	auto edge_10 = mev(pnt_10, glm::vec3(4.35, 1.61, 0), lp_0);
	auto pnt_11 = edge_10->ed;
	auto edge_11 = mev(pnt_11, glm::vec3(3.35, 2.65, 0), lp_0);
	auto pnt_12 = edge_11->ed;
	auto edge_12 = mev(pnt_12, glm::vec3(2.39, 1.60, 0), lp_0);
	auto pnt_13 = edge_12->ed;
	auto edge_13 = mev(pnt_13, glm::vec3(1.53, 2.84, 0), lp_0);
	auto pnt_14 = edge_13->ed;
	auto edge_14 = mev(pnt_14, glm::vec3(2.60, 3.97, 0), lp_0);
	auto pnt_15 = edge_14->ed;
	auto edge_15 = mev(pnt_15, glm::vec3(2.13, 4.53, 0), lp_0);
	auto pnt_16 = edge_15->ed;
	auto edge_16 = mev(pnt_16, glm::vec3(2.86, 5.62, 0), lp_0);
	auto pnt_17 = edge_16->ed;
	auto edge_17 = mev(pnt_17, glm::vec3(1.52, 5.62, 0), lp_0);
	auto lp_1 = mef(pnt_0, edge_17->ed, lp_0);

	auto edge_18 = mev(pnt_4, glm::vec3(4.35, 5.40, 0), lp_1);
	auto pnt_18 = edge_18->ed;
	auto edge_19 = mev(pnt_18, glm::vec3(4.35, 4.16, 0), lp_1);
	auto pnt_19 = edge_19->ed;
	auto edge_20 = mev(pnt_19, glm::vec3(3.57, 3.29, 0), lp_1);
	auto pnt_20 = edge_20->ed;
	auto edge_21 = mev(pnt_20, glm::vec3(3.00, 3.94, 0), lp_1);
	auto pnt_21 = edge_21->ed;
	auto lp_2 = kemr(pnt_4, pnt_18, lp_1);
	auto lp_3 = mef(pnt_18, pnt_21, lp_2);
	kfmrh(lp_0->lface, lp_3->lface);

	auto edge_22 = mev(pnt_9, glm::vec3(4.35, 3.71, 0), lp_1);
	auto pnt_22 = edge_22->ed;
	auto edge_23 = mev(pnt_22, glm::vec3(4.35, 2.01, 0), lp_1);
	auto pnt_23 = edge_23->ed;
	auto edge_24 = mev(pnt_23, glm::vec3(3.59, 2.87, 0), lp_1);
	auto pnt_24 = edge_24->ed;
	lp_2 = kemr(pnt_9, pnt_22, lp_1);
	lp_3 = mef(pnt_22, pnt_24, lp_2);
	kfmrh(lp_0->lface, lp_3->lface);

	auto edge_25 = mev(pnt_13, glm::vec3(2.41, 2.08, 0), lp_1);
	auto pnt_25 = edge_25->ed;
	auto edge_26 = mev(pnt_25, glm::vec3(1.91, 2.81, 0), lp_1);
	auto pnt_26 = edge_26->ed;
	auto edge_27 = mev(pnt_26, glm::vec3(2.80, 3.74, 0), lp_1);
	auto pnt_27 = edge_27->ed;
	auto edge_28 = mev(pnt_27, glm::vec3(3.39, 3.09, 0), lp_1);
	auto pnt_28 = edge_28->ed;
	lp_2 = kemr(pnt_13, pnt_25, lp_1);
	lp_3 = mef(pnt_25, pnt_28, lp_2);
	kfmrh(lp_0->lface, lp_3->lface);

	sweep(S->firstFace, glm::vec3(0, 0, 1), 1.50);
	return S;
	*/
}


void check(Solid* S) {
	int face_cnt = 0, loop_cnt = 0;
	auto face = S->firstFace;
	do {
		std::cout << face_cnt++ << std::endl;
		auto loop = face->firstLoop;
		do {
			std::cout << "   " << loop_cnt++ << std::endl;
			auto edge = loop->firstHalfEdge;
			do {
				std::cout << "    from " << edge->st->pnt[0] << "  " << edge->st->pnt[1] << "  " << edge->st->pnt[2] << "    to " << edge->ed->pnt[0] << "  " << edge->ed->pnt[1] << "  " << edge->ed->pnt[2] << "\n\n\n";
				edge = edge->nextHalfEdge;
			} while (edge != loop->firstHalfEdge);
			loop = loop->nextLoop;
		} while (loop != face->firstLoop);
		face = face->nextFace;
	} while (face != S->firstFace);
	
	/*
	int cnt = 0;
	auto p = S->firstEdge;
	do {
		std::cout << ++cnt << std::endl;
		std::cout << p->l_halfedge->st->pnt[0] << "    " << p->l_halfedge->st->pnt[1] << "    " << p->l_halfedge->st->pnt[2] << std::endl;
		std::cout << p->l_halfedge->ed->pnt[0] << "    " << p->l_halfedge->ed->pnt[1] << "    " << p->l_halfedge->ed->pnt[2] << 
		std::endl<< std::endl <<  std::endl << std::endl  ;
		p = p->nextEdge;
	} while (p != S->firstEdge && p != nullptr);
	*/
}


int InitGLFW() {
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	window = glfwCreateWindow(SCREEN_SIZE.x, SCREEN_SIZE.y, "Euler Operation", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	return 0;
}


void InitController() {
	if (!Controller::Initialize()) {
		std::cout << "Unable to initialize camera!" << std::endl;
	}
	glfwSetKeyCallback(window, Controller::KeyCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


void ReleaseController() {
	Controller::Release();
}




int main() {
	InitGLFW();
	InitController();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2, 0.2, 0.4, 1.0);
	Solid* drawSolid = Read();
	//check(drawSolid);
	GLfloat deltaTime = 0, currentTime = 0;
	GLfloat startTime = glfwGetTime();
	GLfloat lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		GLfloat newTime = glfwGetTime();
		Controller::Movement(newTime - lastTime);
		lastTime = newTime;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		glViewport(0, 0, SCREEN_SIZE.x, SCREEN_SIZE.y);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		gluPerspective(45.0f, SCREEN_SIZE.x / SCREEN_SIZE.y, 0.1f, 100.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity(); 
		glTranslatef(-5.0f, -5.0f, -10.0f);
		glRotatef(Controller::zAngle, 0.0f, 1.0f, 0.0f);
		glRotatef(Controller::yAngle, 1.0f, 0.0f, 1.0f);
		DrawSolid(drawSolid);
		glfwSwapBuffers(window);
	}
	ReleaseController();
	glfwTerminate();
	return 0;
}

