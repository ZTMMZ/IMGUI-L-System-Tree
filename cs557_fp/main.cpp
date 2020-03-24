#include <stdio.h>
#include <string.h>
#include "imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "grammar.h"

#include <glew.h>
#include <glut.h>
#include <GLFW/glfw3.h>
#include "glslprogram.h"
#include "bmptotexture.cpp"
#include <math.h>
#include <vector>
#include <tuple>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#define PI 3.14159265
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description);
GLFWwindow* setup_window(int width, int height, char* title);
void init_imgui(GLFWwindow* window);
void clean_up(GLFWwindow* window);
void setup_glfw(GLFWwindow* window);
void init_list();
void draw_objects();
void main_loop(GLFWwindow* window);

//Inits
GLuint TreeList, PlaneList;
GLSLProgram *TreeTexShader, *MandelbrotShader;
GLuint LeafTex, BarkTex;
std::string grammarResult;
int Mode = 0;

//Mandelbrot
int Iterations= 1;
int Power=2;
int IsMorJ = 0;
ImVec4 img_color_0 = ImVec4(0.f, 0.f, 0.f, 1.f);
ImVec4 img_color_1 = ImVec4(1.f, 1.f, 1.f, 1.f);
ImVec2 CPos = ImVec2(0.3f, 0.5f);
float Gamma = 1.0;
float ColorOffset = 1.0;

//Colors
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
ImVec4 leaf_color = ImVec4(0.95f, 0.55f, 0.60f, 1.00f);
ImVec4 trunk_color = ImVec4(0.45f, 0.05f, 0.60f, 1.00f);

//Mouse Drag
const float MINSCALE = { 0.05f };
GLint width, height;
GLfloat Xrot, Yrot;
GLfloat Xpos, Ypos;
GLfloat Xc, Yc;
GLfloat Scale = 1.f;
bool FirstMouse = true;

//Light constants 
int IsFlat = 0;
float Attenuation = 60.;
int Iter = 7;
ImVec4 Light = ImVec4(6., 14., 20., 1.);
ImVec4 AmbientColor = ImVec4(.09, .19, .30, 1.);
ImVec4 DiffuseColor = ImVec4(.97, .67, .62, 1.);
ImVec4 SpecularColor = ImVec4(1., 1., 1., 1.);

//Gen constants 
ImVec4 UnitDegree = ImVec4(35., 35., 30., 1.);
ImVec2 LeafSize = ImVec2(0.3, 0.46);
float BaseHeight = 2.;
float HeightReduceRate = 0.75;
float BaseRad = 0.1;
float RadReduceRate = 0.5;

//Offset
float OffsetFactor = 0.;
ImVec4 DegreeOffset = ImVec4(5., 5., 5., 0.);
ImVec2 LeafSizeOffset = ImVec2(0.0, 0.0);
float BaseHeightOffset = 1.;
float HeightReduceRateOffset = 0.2;
float BaseRadOffset = 0.02;
float RadReduceRateOffset = 0.1;

//Rule
struct Rule {
	char RuleName[64];
	char RuleStart[32];
	char RulesText[32 * 128];
};
struct Rule CurrentRule;

//Decode and Draw
double getRandOffset(float OffsetFactor) {
	return 2.*OffsetFactor*(rand()*1.0 / RAND_MAX) - OffsetFactor;
}
std::string getGrammar(int iterate, struct Rule ruleData) {
	Grammar* grammar = new Grammar();
	//grammar->setGrammarName("Tree");
	//grammar->addGeneration('S', "F[$^X][%*X][%&X]");
	//grammar->addGeneration('X', "F[%^D][$&D][$/D][%*D]");
	//grammar->addGeneration('X', "F[%&D][$*D][$/D][%^D]");
	//grammar->addGeneration('D', "F[$^X][%*FX][%&X]");
	//grammar->addGeneration('D', "F[$^FX][%*X][%&X]");
	//grammar->addGeneration('D', "F[$^X][%*X][%&FX]");
	//grammar->setStart("S");
	
	std::vector<std::tuple<char, std::string>> Rules;
	char tmp[32 * 128] = "";
	strcpy(tmp, ruleData.RulesText);
	const char *sep = "\n";
	char *RuleText;
	RuleText = strtok(tmp, sep);
	while (RuleText) {
		char L = RuleText[0];
		std::string R = RuleText;
		R = R.substr(2);
		Rules.push_back(std::make_tuple(L, R));
		RuleText = strtok(NULL, sep);
	}

	grammar->setGrammarName(ruleData.RuleName);
	for (std::vector<std::tuple<char, std::string>>::const_iterator iter = Rules.cbegin(); iter != Rules.cend(); iter++) {
		grammar->addGeneration(std::get<0>(*iter), std::get<1>(*iter));
	}
	grammar->setStart(ruleData.RuleStart);
	grammar->iterateFor(iterate);
	std::string str = std::string(grammar->getResult());
	grammar->clear();
	delete(grammar);
	return str;
}
void load2DTexture(GLuint *tex, char *fileName, int width, int height, bool alpha) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	int level = 0, border = 0;
	unsigned char *Texture = BmpToTexture(fileName, &width, &height, alpha);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, Texture);
}
void drawTrunk(float height, float radiusB, float radiusA) {
	glColor3f(trunk_color.x, trunk_color.y, trunk_color.z);
	int i;
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < 360; i += 45) {
		float rad = (float)i / 180 * PI;
		glTexCoord2f((float)i / 360, height*2.);
		glNormal3f(cos(rad)*radiusB, height, -sin(rad)*radiusB);
		glVertex3f(cos(rad)*radiusB, height, -sin(rad)*radiusB);
		glTexCoord2f((float)i / 360, 0.);
		glNormal3f(cos(rad)*radiusA, 0, -sin(rad)*radiusA);
		glVertex3f(cos(rad)*radiusA, 0, -sin(rad)*radiusA);
	}
	glTexCoord2f((float)i / 360, height*2.);
	glNormal3f(cos(0)*radiusB, height, -sin(0)*radiusB);
	glVertex3f(cos(0)*radiusB, height, -sin(0)*radiusB);
	glTexCoord2f((float)i / 360, 0.);
	glNormal3f(cos(0)*radiusA, 0, -sin(0)*radiusA);
	glVertex3f(cos(0)*radiusA, 0, -sin(0)*radiusA);
	glEnd();
}
void drawLine(float height) {
	glLineWidth(4);
	glBegin(GL_LINES);
		glVertex3f(0., height, 0.);
		glVertex3f(0., 0., 0.);
	glEnd();
}
void drawLeaf(float leafWidth, float leafHeight) {
	glColor3f(leaf_color.x, leaf_color.y, leaf_color.z);
	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.);
	glVertex3f(-leafWidth / 2., 0., 0.);
	glTexCoord2f(0., 1.);
	glVertex3f(-leafWidth / 2., leafHeight, 0.);
	glTexCoord2f(1., 1.);
	glVertex3f(leafWidth / 2., leafHeight, 0.);
	glTexCoord2f(1., 0.);
	glVertex3f(leafWidth / 2., 0., 0.);
	glEnd();
}
void drawTree(std::string grammer) {
	int scaleTime = 0;
	ImVec4 _UnitDegree;
	ImVec2 _LeafSize;
	float _BaseHeight, _HeightReduceRate, _BaseRad, _RadReduceRate;

	for (int i = 0; i<grammer.size(); i++) {
		//offset
		_UnitDegree.x = UnitDegree.x + getRandOffset(OffsetFactor)*DegreeOffset.x;
		_UnitDegree.y = UnitDegree.y + getRandOffset(OffsetFactor)*DegreeOffset.y;
		_UnitDegree.z = UnitDegree.z + getRandOffset(OffsetFactor)*DegreeOffset.z;
		_LeafSize.x = LeafSize.x + getRandOffset(OffsetFactor)*LeafSizeOffset.x;
		_LeafSize.y = LeafSize.y + getRandOffset(OffsetFactor)*LeafSizeOffset.y;
		_BaseHeight = BaseHeight + getRandOffset(OffsetFactor)*BaseHeightOffset;
		_HeightReduceRate = HeightReduceRate + getRandOffset(OffsetFactor)*HeightReduceRateOffset;
		_BaseRad = BaseRad + getRandOffset(OffsetFactor)*BaseRadOffset;
		_RadReduceRate = RadReduceRate + getRandOffset(OffsetFactor)*RadReduceRateOffset;

		//decode
		char ch = grammer[i];
		switch (ch) {
		case 'K':
			drawLine(BaseHeight* (3. / pow(3., Iter)));
			glTranslatef(0., BaseHeight* (3. / pow(3., Iter)), 0.);
			break;
		case 'L':
			drawLine(_BaseHeight);
			glTranslatef(0., _BaseHeight, 0.);
			glScalef(_HeightReduceRate, _HeightReduceRate, _HeightReduceRate);
			break;
		case 'C':
			drawLine(_BaseHeight);
			glScalef(_HeightReduceRate, _HeightReduceRate, _HeightReduceRate);
			break;
		case 'F':
			glActiveTexture(GL_TEXTURE0);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, BarkTex);
			glDisable(GL_TEXTURE_2D);
			TreeTexShader->SetUniformVariable("uTex", 0);
			drawTrunk(_BaseHeight, _BaseRad*_RadReduceRate, _BaseRad);
			glTranslatef(0., _BaseHeight, 0.);
			glScalef(_HeightReduceRate, _HeightReduceRate, _HeightReduceRate);
			break;
		case '^':
			glRotatef(_UnitDegree.x, 1., 0., 0.);
			break;
		case '&':
			glRotatef(-_UnitDegree.x, 1., 0., 0.);
			break;
		case '$':
			glRotatef(_UnitDegree.y, 0., 1., 0.);
			break;
		case '%':
			glRotatef(-_UnitDegree.y, 0., 1., 0.);
			break;
		case '*':
			glRotatef(_UnitDegree.z, 0., 0., 1.);
			break;
		case '/':
			glRotatef(-_UnitDegree.z, 0., 0., 1.);
			break;
		case '[':
			glPushMatrix();
			scaleTime++;
			break;
		case ']':
			glPopMatrix();
			scaleTime--;
			break;
		case 'D':
		case 'X':
			glActiveTexture(GL_TEXTURE0);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, LeafTex);
			glDisable(GL_TEXTURE_2D);
			TreeTexShader->SetUniformVariable("uTex", 0);
			glPushMatrix();
			glScalef(1. / pow(HeightReduceRate, scaleTime), 1. / pow(HeightReduceRate, scaleTime), 1. / pow(HeightReduceRate, scaleTime));
			drawLeaf(_LeafSize.x, _LeafSize.y);
			glPopMatrix();
			break;
		default:
			break;
		}
	}
	
}

//Mouse Drag
void cursor_drag_callback(GLFWwindow* window, double x, double y) {
	if (ImGui::IsAnyWindowFocused()) return;
	if (FirstMouse) {
		Xpos = x;
		Ypos = y;
		FirstMouse = false;
	}
	Xc += 0.001 * (x - Xpos);
	Yc -= 0.001 * (y - Ypos);
	Xpos = x;
	Ypos = y;
	return;
}

void cursor_rotate_callback(GLFWwindow* window, double x, double y) {
	if (ImGui::IsAnyWindowFocused()) return;
	if (FirstMouse) {
		Xpos = x;
		Ypos = y;
		FirstMouse = false;
	}
	Xrot -= fmod(0.1 * (y - Ypos), 360);
	Yrot -= fmod(0.1 * (x - Xpos), 360);
	Xpos = x;
	Ypos = y;
	return;
}
void cursor_reset_callback(GLFWwindow* window, double x, double y) {
	Xrot = 0;
	Yrot = 0;
	Xpos = x;
	Ypos = y;
	Scale = 1.;
	return;
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
		glfwSetCursorPosCallback(window, cursor_rotate_callback);
	} else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
		FirstMouse = true;
		glfwSetCursorPosCallback(window, NULL);
	} else if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_MIDDLE) {
		glfwSetCursorPosCallback(window, cursor_reset_callback);
	}
	else if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT) {
		glfwSetCursorPosCallback(window, cursor_drag_callback);
	}
	else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_RIGHT) {
		FirstMouse = true;
		glfwSetCursorPosCallback(window, NULL);
	}
	else{
		glfwSetCursorPosCallback(window, NULL);
	}

	return;
}
void scroll_callback(GLFWwindow* window, double x, double y) {
	Scale += 0.05*y;
	return;
}

//Init and Setting
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
GLFWwindow* setup_window(int width, int height, char* title) {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return NULL;
	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL)
		return NULL;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	return window;
}
void init_imgui(GLFWwindow* window) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL2_Init();
}
void clean_up(GLFWwindow* window) {
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}
void setup_glfw(GLFWwindow* window) {
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
}
void init_list() {
	switch (Mode) {
	case 0:
		PlaneList = glGenLists(1);
		glNewList(PlaneList, GL_COMPILE);
		glBegin(GL_POLYGON);
			glVertex2f(-0.5f, 0.5f);
			glTexCoord2d(0, 1);
			glVertex2f(-0.5f, -0.5f);
			glTexCoord2d(0, 0);
			glVertex2f(0.5f, -0.5f);
			glTexCoord2d(1, 0);
			glVertex2f(0.5f, 0.5f);
			glTexCoord2d(1, 1);
		glEnd();
		glEndList();
		break;
	case 1:
		load2DTexture(&LeafTex, "Textures/leaf.bmp", 256, 256, true);
		load2DTexture(&BarkTex, "Textures/bark.bmp", 256, 256, false);
		grammarResult = getGrammar(Iter,CurrentRule);
		TreeList = glGenLists(1);
		glNewList(TreeList, GL_COMPILE);
		drawTree(grammarResult);
		glEndList();
		break;
	default:
		break;
	}
}
void init_glsl() {
	#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	#endif
	TreeTexShader = new GLSLProgram();
	TreeTexShader->Create("Shader/treeTexShader.vert", "Shader/treeTexShader.frag");

	MandelbrotShader = new GLSLProgram();
	MandelbrotShader->Create("Shader/mandelbrot.vert", "Shader/mandelbrot.frag");
}
void setup_ext() {
	// Translucent effect, enable the color blend and anti-aliasing
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST); // Make round points, not square points  
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);  // Antialias the lines  
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	// Unitized the normal vectors
	glEnable(GL_NORMALIZE);
	srand((int)time(NULL));
}
void setup_tree_glsl() {

	switch (Mode) {
	case 0:
		MandelbrotShader->Use();
		MandelbrotShader->SetUniformVariable("uIterations", Iterations);
		MandelbrotShader->SetUniformVariable("uPower", Power);
		MandelbrotShader->SetUniformVariable("uMORJ", IsMorJ); 
		MandelbrotShader->SetUniformVariable("uColor0", img_color_0.x, img_color_0.y, img_color_0.z);
		MandelbrotShader->SetUniformVariable("uColor1", img_color_1.x, img_color_1.y, img_color_1.z);
		MandelbrotShader->SetUniformVariable("uCX", CPos.x);
		MandelbrotShader->SetUniformVariable("uCY", CPos.y);
		MandelbrotShader->SetUniformVariable("uGamma", Gamma);
		MandelbrotShader->SetUniformVariable("uColorOffset", ColorOffset);

		break;
	case 1:
		TreeTexShader->Use();
		TreeTexShader->SetUniformVariable("uKa", float(0.5));
		TreeTexShader->SetUniformVariable("uKd", float(0.35));
		TreeTexShader->SetUniformVariable("uKs", float(0.3));
		TreeTexShader->SetUniformVariable("uSpecularColor", SpecularColor.x, SpecularColor.y, SpecularColor.z, float(1.0));
		TreeTexShader->SetUniformVariable("uAmbientColor", AmbientColor.x, AmbientColor.y, AmbientColor.z, float(1.0));
		TreeTexShader->SetUniformVariable("uDiffuseColor", DiffuseColor.x, DiffuseColor.y, DiffuseColor.z, float(1.0));
		TreeTexShader->SetUniformVariable("uAttenuation", Attenuation);
		TreeTexShader->SetUniformVariable("uShininess", float(10.));
		TreeTexShader->SetUniformVariable("uLightX", Light.x);
		TreeTexShader->SetUniformVariable("uLightY", Light.y);
		TreeTexShader->SetUniformVariable("uLightZ", Light.z);
		TreeTexShader->SetUniformVariable("uFlat", IsFlat);
		break;
	default:
		break;
	}
}

void draw_objects() {
	glLoadIdentity();
	glRotated((GLfloat)Yrot, 0., 1., 0.);
	glRotated((GLfloat)Xrot, 1., 0., 0.);
	if (Scale < MINSCALE)
		Scale = MINSCALE;
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
	glTranslatef(Xc, Yc, 0.);

	switch (Mode) {
	case 0:
		glPushMatrix();
		glTranslatef(-0.25, 0., 0.);
		glScalef(1, 1.75, 1);
		glCallList(PlaneList);
		glPopMatrix();
		break;
	case 1:
		glPushMatrix();
		glTranslatef(0., -0.5, 0.);
		glScalef(0.1, 0.175, 0.1);
		glCallList(TreeList);
		glPopMatrix();
		break;
	default:
		break;
	}

}
void init_form() {
	ImGui::Begin("Console Panel");          
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (ImGui::Button("Quasi")) Mode = 0;
	ImGui::SameLine();
	if (ImGui::Button("L-SYSTEM")) Mode = 1;

	switch (Mode) {
	case 0:
		ImGui::ColorEdit3("color 0", (float*)&img_color_0);
		ImGui::ColorEdit3("color 1", (float*)&img_color_1);
		ImGui::SliderInt("iteration", &Iterations, 1, 500);
		ImGui::SliderInt("power", &Power, 1, 10); 
		ImGui::SliderFloat("gamma", &Gamma, .0f, 10.0f);
		ImGui::SliderFloat("color offset", &ColorOffset, -5.0f, 5.0f);
		ImGui::DragFloat2("c", (float*)&CPos, 0.01f, -2.0f, 2.0f, "%.2f");
		if (ImGui::Button("MandelBrot")) IsMorJ = 0;
		ImGui::SameLine(); 
		if (ImGui::Button("Julia")) IsMorJ = 1;
		break;
	case 1:
		ImGui::Text("Light Setting");
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		ImGui::DragFloat3("light direction", (float*)&Light, 0.5f, -100.0f, 100.0f, "%.1f");
		ImGui::ColorEdit3("ambient color", (float*)&AmbientColor);
		ImGui::ColorEdit3("diffuseColor color", (float*)&DiffuseColor);
		ImGui::ColorEdit3("specular color", (float*)&SpecularColor);
		ImGui::Text("Generating Factors");
		ImGui::SliderInt("iteration", &Iter, 1.0f, 8.0f);
		ImGui::DragFloat3("unit degree", (float*)&UnitDegree, 0.1f, -180.0f, 180.0f, "%.1f");
		ImGui::DragFloat2("leaf size", (float*)&LeafSize, 0.01f, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("base height", &BaseHeight, 0.0f, 10.0f);
		ImGui::SliderFloat("height reduce rate", &HeightReduceRate, 0.0f, 1.0f);
		ImGui::SliderFloat("base radius", &BaseRad, 0.0f, 1.0f);
		ImGui::SliderFloat("radius reduce rate", &RadReduceRate, 0.0f, 1.0f);

		ImGui::Text("Offset");

		ImGui::SliderFloat("offset factor", &OffsetFactor, 0.0f, 1.0f);
		ImGui::DragFloat3("degree offset", (float*)&DegreeOffset, 0.1f, -180.0f, 180.0f, "%.1f");
		ImGui::DragFloat2("leaf size offset", (float*)&LeafSizeOffset, 0.01f, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("base height offset", &BaseHeightOffset, 0.0f, 10.0f);
		ImGui::SliderFloat("height reduce offset", &HeightReduceRateOffset, 0.0f, 1.0f);
		ImGui::SliderFloat("base radius offset", &BaseRadOffset, 0.0f, 1.0f);
		ImGui::SliderFloat("radius reduce offset", &RadReduceRateOffset, 0.0f, 1.0f);

		ImGui::Text("Rules");
		struct Rule KochRule;
		strcpy(KochRule.RuleName, "Koch");
		strcpy(KochRule.RuleStart, "O");
		strcpy(KochRule.RulesText, "O, K////K////K\n"
			"K, K**K////K**K");

		struct Rule TreeRule;
		strcpy(TreeRule.RuleName, "Tree");
		strcpy(TreeRule.RuleStart, "S");
		strcpy(TreeRule.RulesText, "S, F[$^X][%*X][%&X]\n"
			"X, F[%^D][$&D][$/D][%*D]\n"
			"X, F[%&D][$*D][$/D][%^D]\n"
			"D, F[$^X][%*FX][%&X]\n"
			"D, F[$^FX][%*X][%&X]\n"
			"D, F[$^X][%*X][%&FX]");

		if (ImGui::Button("Koch")) CurrentRule = KochRule;
		ImGui::SameLine();
		if (ImGui::Button("Tree")) CurrentRule = TreeRule;

		ImGui::InputText("rule name", CurrentRule.RuleName, IM_ARRAYSIZE(CurrentRule.RuleName));
		ImGui::InputText("rule start", CurrentRule.RuleStart, IM_ARRAYSIZE(CurrentRule.RuleStart));
		ImGui::InputTextMultiline("##rules", CurrentRule.RulesText, IM_ARRAYSIZE(CurrentRule.RulesText), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);

		if (ImGui::Button("Generate")) init_list();
		break;
	default:
		break;
	}
	

	ImGui::End();
}

void main_loop(GLFWwindow* window) {
	glfwPollEvents();
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	init_form();

	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setup_tree_glsl();
	draw_objects();

	switch (Mode) {
	case 0:
		MandelbrotShader->Use(0);
		break;
	case 1:
		TreeTexShader->Use(0);
		break;
	default:
		break;
	}

	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);
}
int main(int, char**) {
	GLFWwindow* window = setup_window(1900,1000, "Fractal");
	if (window ==NULL) return 0;
	init_imgui(window);
	setup_glfw(window);
	setup_ext();
	init_glsl();
	init_list();
    while (!glfwWindowShouldClose(window)) { main_loop(window); }
	clean_up(window);
    return 0;
}