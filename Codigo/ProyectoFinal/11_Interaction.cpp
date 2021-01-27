// Iluminación y texturizado

#include <iostream>
#include <stdlib.h>

// GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
// https://glad.dav1d.de/
#include <glad/glad.h>

// GLFW: https://www.glfw.org/
#include <GLFW/glfw3.h>

// GLM: OpenGL Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Model loading classes
#include <shader_m.h>
#include <camera.h>
#include <model.h>

#include <iostream>

// Definición de callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// Tamaño en pixeles de la ventana
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Definición de cámara (posición en XYZ)
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));

// Controladores para el movimiento del mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables para la velocidad de reproducción
// de la animación
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float elapsedTime = 0.0f;

glm::vec3 position(0.0f,0.0f, 0.0f);
glm::vec3 forwardView(0.0f, 0.0f, 1.0f);
float     scaleV = 0.005f;
float     rotateCharacter = 0.0f;

float     rotatePuerta = 0.0f;
float     trasladeRopero = 0.0f;
float	  trasladeCajones = 0.0f;
float	  trasladeRopero2 = 0.0f;
float	  trasladeBasura = 0.0f;
float	  trasladeSilla = 0.0f;
float	  trasladeJarron = 0.0f;

// Entrada a función principal
int main()
{
	// Inicialización de GLFW

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creación de la ventana con GLFW
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FBX Animation with OpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Ocultar el cursor mientras se rota la escena
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: Cargar todos los apuntadores
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Activación de buffer de profundidad
	glEnable(GL_DEPTH_TEST);

	// Compilación y enlace de shaders
	Shader ourShader("shaders/vertex_skinning-IT.vs", "shaders/fragment_skinning-IT.fs");
	Shader staticShader("shaders/vertex_simple.vs", "shaders/fragment_simple.fs");
	Shader cubemapShader("shaders/vertex_cubemap.vs", "shaders/fragment_cubemap.fs");

	// Máximo número de huesos: 100
#define MAX_RIGGING_BONES 100
	ourShader.setBonesIDs(MAX_RIGGING_BONES);
	
	// Carga la información del modelo
	Model character("models/interaction/Persona1.fbx");
	Model house("models/interaction/CasaAlternativa.fbx");
	Model skybox("models/interaction/Skybox.fbx");
	Model Ropero("models/interaction/ropero1.fbx");
	Model Puerta("models/interaction/Puerta2.fbx");
	Model cajones("models/interaction/cajones.fbx");
	Model ropero2("models/interaction/ropero2.fbx");
	Model basura("models/interaction/basura.fbx");
	Model silla("models/interaction/silla.fbx");
	//Model jarron("models/interaction/jarron.fbx");
	// Dibujar en malla de alambre
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Pose inicial del modelo
	glm::mat4 gBones[MAX_RIGGING_BONES];
	
	// time, arrays
	character.SetPose(0.0f, gBones);

	float fps = (float)character.getFramerate();
	int keys = (int)character.getNumFrames();

	int animationCount = 0;

	// Loop de renderizado
	while (!glfwWindowShouldClose(window))
	{
		// Cálculo del framerate
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		elapsedTime += deltaTime;
		if ( elapsedTime > 1.0f/fps ) {
			animationCount++;
			if (animationCount > keys-1) {
				animationCount = 0;
			}
			// Configuración de la pose en el instante t
			character.SetPose((float)animationCount, gBones);
			elapsedTime = 0.0f;
		}
		
		// Procesa la entrada del teclado o mouse
		processInput(window);

		// Renderizado R - G - B - A
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Objeto animado
		{
			// Activación del shader del personaje
			ourShader.use();

			// Aplicamos transformaciones de proyección y cámara (si las hubiera)
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			ourShader.setMat4("projection", projection);
			ourShader.setMat4("view", view);

			// Aplicamos transformaciones del modelo
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, position); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down
			
			ourShader.setMat4("model", model);

			ourShader.setMat4("gBones", MAX_RIGGING_BONES, gBones);

			// Dibujamos el modelo
			character.Draw(ourShader);
		}
		// Desactivamos el shader actual
		glUseProgram(0);



		// Objeto estático (casa)
		{
			// Activamos el shader del plano
			staticShader.use();

			// Activamos para objetos transparentes
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Aplicamos transformaciones de proyección y cámara (si las hubiera)
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			staticShader.setMat4("projection", projection);
			staticShader.setMat4("view", view);

			// Aplicamos transformaciones del modelo
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
			staticShader.setMat4("model", model);

			house.Draw(staticShader);
		}

		glUseProgram(0);

		// Skybox
		{
			// Activamos el shader del plano
			cubemapShader.use();

			// Aplicamos transformaciones de proyección y cámara (si las hubiera)
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			cubemapShader.setMat4("projection", projection);
			cubemapShader.setMat4("view", view);

			// Aplicamos transformaciones del modelo
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 200.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1000.0f, 1000.0f, 1000.0f));	// it's a bit too big for our scene, so scale it down
			cubemapShader.setMat4("model", model);

			skybox.Draw(cubemapShader);

		}

		glUseProgram(0);

		{  //PUERTA
			// Activamos el shader estático
			staticShader.use();

			// Activamos para objetos transparentes
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Aplicamos transformaciones de proyección y cámara (si las hubiera)
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			staticShader.setMat4("projection", projection);
			staticShader.setMat4("view", view);

			// Aplicamos transformaciones del modelo
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.6f, 0.1f, -4.2f)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(rotatePuerta), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
			staticShader.setMat4("model", model);

			Puerta.Draw(staticShader);

		}

		glUseProgram(0);

		{
			//Ropero
			// Activamos el shader estático
			staticShader.use();

			// Activamos para objetos transparentes
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Aplicamos transformaciones de proyección y cámara (si las hubiera)
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			staticShader.setMat4("projection", projection);
			staticShader.setMat4("view", view);

			// Aplicamos transformaciones del modelo
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::translate(model, glm::vec3(trasladeRopero,0.0f , 0.0f)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
			staticShader.setMat4("model", model);

			Ropero.Draw(staticShader);
		}
		glUseProgram(0);
		
		{
			//Cajones
			// Activamos el shader estático
			staticShader.use();

			// Activamos para objetos transparentes
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Aplicamos transformaciones de proyección y cámara (si las hubiera)
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			staticShader.setMat4("projection", projection);
			staticShader.setMat4("view", view);

			// Aplicamos transformaciones del modelo
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::translate(model, glm::vec3(trasladeCajones, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
			staticShader.setMat4("model", model);

			cajones.Draw(staticShader);
		}
		glUseProgram(0);
		{
			//Ropero2
			// Activamos el shader estático
			staticShader.use();

			// Activamos para objetos transparentes
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Aplicamos transformaciones de proyección y cámara (si las hubiera)
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			staticShader.setMat4("projection", projection);
			staticShader.setMat4("view", view);

			// Aplicamos transformaciones del modelo
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, trasladeRopero2)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
			staticShader.setMat4("model", model);

			ropero2.Draw(staticShader);
		}
		glUseProgram(0);
		{
			//Basura
			// Activamos el shader estático
			staticShader.use();

			// Activamos para objetos transparentes
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Aplicamos transformaciones de proyección y cámara (si las hubiera)
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			staticShader.setMat4("projection", projection);
			staticShader.setMat4("view", view);

			// Aplicamos transformaciones del modelo
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::translate(model, glm::vec3(trasladeBasura, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
			staticShader.setMat4("model", model);

			basura.Draw(staticShader);
		}
		glUseProgram(0);

		{
			//Silla
			// Activamos el shader estático
			staticShader.use();

			// Activamos para objetos transparentes
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Aplicamos transformaciones de proyección y cámara (si las hubiera)
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			staticShader.setMat4("projection", projection);
			staticShader.setMat4("view", view);

			// Aplicamos transformaciones del modelo
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, trasladeSilla)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
			staticShader.setMat4("model", model);

			silla.Draw(staticShader);
		}
		glUseProgram(0);
		/*{
			//Jarrón
			// Activamos el shader estático
			staticShader.use();

			// Activamos para objetos transparentes
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Aplicamos transformaciones de proyección y cámara (si las hubiera)
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			staticShader.setMat4("projection", projection);
			staticShader.setMat4("view", view);

			// Aplicamos transformaciones del modelo
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::translate(model, glm::vec3(trasladeJarron, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
			staticShader.setMat4("model", model);

			jarron.Draw(staticShader);
		}

		glUseProgram(0); */

		// glfw: swap buffers 
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: Terminamos el programa y liberamos memoria
	glfwTerminate();
	return 0;
}

// Procesamos entradas del teclado
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	// Character movement
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		position = position + scaleV * forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		position = position - scaleV * forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		rotateCharacter += 0.5f;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		forwardView = glm::vec3(viewVector);
		forwardView = glm::normalize(forwardView);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		rotateCharacter -= 0.5f;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		forwardView = glm::vec3(viewVector);
		forwardView = glm::normalize(forwardView);
	}
	//Puerta
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
	{
		rotatePuerta += 0.20f;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		rotatePuerta -= 0.20f;
	}
	//Ropero1
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		trasladeRopero -= 0.02f;
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		trasladeRopero += 0.02f;
	}
	//Cajones
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		trasladeCajones += 0.02f;
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		trasladeCajones -= 0.02f;
	}
	//Basura
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		trasladeBasura += 0.02f;
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		trasladeBasura -= 0.02f;
	}
	//Ropero 2
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		trasladeRopero2 -= 0.02f;
	}
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		trasladeRopero2 += 0.02f;
	}
	//Silla
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		trasladeSilla += 0.02f;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		trasladeSilla -= 0.02f;
	}
	/*//Jarron
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		trasladeJarron += 0.02f;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		trasladeJarron -= 0.02f;
	}*/
}

// glfw: Actualizamos el puerto de vista si hay cambios del tamaño
// de la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: Callback del movimiento y eventos del mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; 

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: Complemento para el movimiento y eventos del mouse
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}
