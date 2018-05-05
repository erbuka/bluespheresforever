#include "GameRendering.h"

GameRendering::GameRendering(void)
{
	this->InitBuffers();
}


GameRendering::~GameRendering(void)
{
}

void GameRendering::Configure(const GameConfig& gameConfig) {
	this->quality = gameConfig.quality;
}


void GameRendering::RenderWorldFloor() {
	if(this->quality != 2)
		this->DrawVAO(this->worldVAOLow);
	else if(this->quality == 2)
		this->DrawVAO(this->worldVAOHigh);
}

void GameRendering::RenderSonicRun(Shader * shader, GLuint frame) {
	this->sonicRun->DrawAnimation(shader, frame);
}

void GameRendering::RenderSonicJump(Shader * shader) {
	this->sonicJump->DrawAnimation(shader, 0);
}

void GameRendering::RenderSonicStand(Shader * shader) {
	this->sonicStand->DrawAnimation(shader, 0);
}

void GameRendering::RenderSphere() {
	if(this->quality != 2)
		this->DrawVAO(this->sphereLowVAO);
	else if(this->quality == 2)
		this->DrawVAO(this->sphereHighVAO);
}
void GameRendering::RenderRing() {
	this->DrawVAO(this->ringVAO);
}

void GameRendering::RenderEmerald() {
	this->DrawVAO(this->emeraldVAO);
}

void GameRendering::RenderSky() {
	this->DrawVAO(this->skyVAO, GL_POINTS);
}


void GameRendering::DrawVAO(VertexArrayObject * vao) {
	glBindVertexArray(vao->getID());
	glDrawElements(GL_TRIANGLES, vao->getElementsCount(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void GameRendering::DrawVAO(VertexArrayObject * vao, GLenum mode) {
	glBindVertexArray(vao->getID());
	glDrawElements(mode, vao->getElementsCount(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void GameRendering::InitBuffers() {

	XBO * obj;

	// Spheres
	
	obj = FromFile(Global::GetPath(Global::MODELS_DIR, "sphere.xbo"));

	this->sphereHighVAO = Global::CreateVertexArray(obj, 0);

	this->sphereHighVAO->enableVertexAttribute(0);
	this->sphereHighVAO->enableVertexAttribute(1);
	this->sphereHighVAO->enableVertexAttribute(2);
	
	delete obj;

	obj = FromFile(Global::GetPath(Global::MODELS_DIR, "sphereLow.xbo"));

	this->sphereLowVAO = Global::CreateVertexArray(obj, 0);

	this->sphereLowVAO->enableVertexAttribute(0);
	this->sphereLowVAO->enableVertexAttribute(1);
	this->sphereLowVAO->enableVertexAttribute(2);
	
	delete obj;

	// Rings

	obj = FromFile(Global::GetPath(Global::MODELS_DIR, "ring.xbo"));
	this->ringVAO = Global::CreateVertexArray(obj, 0);

	this->ringVAO->enableVertexAttribute(0);
	this->ringVAO->enableVertexAttribute(1);
	this->ringVAO->enableVertexAttribute(2);

	delete obj;

	// Emerald

	obj = FromFile(Global::GetPath(Global::MODELS_DIR, "emerald.xbo"));
	this->emeraldVAO = Global::CreateVertexArray(obj, 0);

	this->emeraldVAO->enableVertexAttribute(0);
	this->emeraldVAO->enableVertexAttribute(1);
	this->emeraldVAO->enableVertexAttribute(2);

	delete obj;

	// World plane
	this->worldVAOHigh = new WorldVertexArray(10, 4);
	this->worldVAOLow = new WorldVertexArray(10, 2);

	// Sky
	this->skyVAO = new SkyVertexArray(2500, vec2(15, 60), vec2(5, 10), 10);

	// Sonic

	vector<string> runFrames, jumpFrames, standFrames;

	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic0.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic1.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic2.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic3.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic4.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic5.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic6.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic7.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic8.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic9.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic10.xbo"));
	runFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonic11.xbo"));

	jumpFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonicJump.xbo"));

	standFrames.push_back(Global::GetPath(Global::MODELS_DIR, "sonicStand.xbo"));

	this->sonicRun = new Sonic(runFrames);
	this->sonicJump = new Sonic(jumpFrames);
	this->sonicStand = new Sonic(standFrames);
}


SkyVertexArray * GameRendering::GetSkyVertexArray() { return this->skyVAO; }