#include <iostream>
#include <flgl.h>
#include <vector>
#include "../lib/flgl/src/Verticies.h"
#include "../lib/flgl/src/tools/Camera.h"
#include "../lib/flgl/src/tools/flgl_math.h"
using std::vector;
using namespace glm;
static Window& window = flgl::window;

static vector<Vt_classic> qverts = { 
	{{-1.,-1., 0.}, {0.,0.}},
	{{-1., 1., 0.}, {0.,1.}},
	{{ 1., 1., 0.}, {1.,1.}},
	{{ 1.,-1., 0.}, {1.,0.}} 
};

static vector<uint32_t> qelems = {0, 1, 2,   0, 3, 2};
static VertexArray quad;
void setup_quad_vao() {
	quad.create();
	VertexBuffer<Vt_classic> vbo;
	ElementBuffer ibo;
	quad.bind();
	vbo.create();
	vbo.bind();
	quad.attach(vbo);
	vbo.buffer(qverts);

	ibo.create();
	ibo.bind();
	ibo.buffer_data(qelems);
	quad.unbind();
	vbo.destroy();
	ibo.destroy();
}

void createCylinder(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<uint32_t>& indices, int numCircleVertices, float height) {
    float radius = 1.0f; // You can modify this as needed
    float halfHeight = height / 2.0f;

    // Clear vectors
    vertices.clear();
    indices.clear();

    // Generate vertices
    for (int i = 0; i < 2; ++i) {
        float y = i * height - halfHeight;
        for (int j = 0; j < numCircleVertices; ++j) {
            float angle = 2.0f * M_PI * j / numCircleVertices;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back(glm::vec3(x, y, z));
        }
    }

    // Generate indices for the side surface
    for (int i = 0; i < numCircleVertices; ++i) {
        int next = (i + 1) % numCircleVertices;

        // Two triangles per quad
        indices.push_back(i);
        indices.push_back(i + numCircleVertices);
        indices.push_back(next + numCircleVertices);

        indices.push_back(i);
        indices.push_back(next + numCircleVertices);
        indices.push_back(next);
    }


    // Generate vertices and normals
    for (int i = 0; i < 2; ++i) {
        float y = i * height - halfHeight;
        for (int j = 0; j < numCircleVertices; ++j) {
            float angle = 2.0f * M_PI * j / numCircleVertices;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back(glm::vec3(x, y, z));

            // Compute normal (without considering y-component)
            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
            normals.push_back(normal);
        }
    }

}

struct rbfVt {
	vec3 pos, normal;
};

template <>
void VertexBuffer<rbfVt>::attach_to_vao(VertexArray const& vao) const {
	vao.bind();
	this->bind();
	vao.attrib(0,							// layout index
			   3, GL_FLOAT,					// dimension and repr
			   sizeof(rbfVt), 				// size of vertex
			   offsetof(rbfVt,pos));		// offset of member

	vao.attrib(1,							// layout index
			   3, GL_FLOAT,					// dimension and repr
			   sizeof(rbfVt), 				// size of vertex
			   offsetof(rbfVt,normal));		// offset of member
}

static VertexArray cyl;
static void setup_cyl() {
	vector<vec3> vvec;
	vector<vec3> nvec;
	vector<uint32_t> ivec;
	createCylinder(vvec, nvec, ivec, 16, 2);
	vector<rbfVt> rbfVec;
	for (uint32_t i = 0; i < vvec.size(); i++) {
		rbfVec.push_back({{vvec[i]}, {nvec[i]}});
	}
	cyl.create();
	cyl.bind();
	VertexBuffer<rbfVt> vbo;
	vbo.create();
	cyl.attach(vbo);
	vbo.buffer(rbfVec);

	ElementBuffer ibo;
	ibo.create(); ibo.bind();
	ibo.buffer_data(ivec);
	cyl.unbind();
	vbo.destroy();
	ibo.destroy();
}


static PerspectiveCamera cam(vec3(0, 0, -1), vec3(0, 0, 1), vec3(0, 1, 0), 0.001, 10000, 60);
static glm::mat4 qmmat = genModelMat3d(vec3(0, -1, 10), vec3(90, 0, 0), vec3(16*3, 20, 1));
static Texture grass;

static mat4 cylmat = genModelMat3d(vec3(0., 0., 10.), vec3(0.), vec3(0.5, 2, 0.5));

static int base_index = 0;
static mat4 cylmat_first_base = genModelMat3d(vec3(1., -1., 10.), vec3(0.), vec3(0.25, 0.05, 0.25));
static mat4 cylmat_second_base = genModelMat3d(vec3(1., -1., 10.), vec3(0.), vec3(0.25, 0.05, 0.25));
static mat4 cylmat_third_base = genModelMat3d(vec3(1., -1., 10.), vec3(0.), vec3(0.25, 0.05, 0.25));
static mat4 cylmat_home_base = genModelMat3d(vec3(1., -1., 10.), vec3(0.), vec3(0.25, 0.05, 0.25));

int main() {

	flgl::gl.init();
	flgl::config.set_shader_path("shad/");
	flgl::config.set_asset_path("assets/");
	flgl::window.create("rbf baseball", 1280, 1280);
	flgl::gl.enable_depth_test(true);
	grass = Texture::from_file("grass", true);

	setup_quad_vao();
	setup_cyl();
	Shader grass_shader = Shader::from_source("vert", "grass_frag");
	Shader cyl_shader = Shader::from_source("cyl_vert", "cylinder");
	cam.update();
	bool wf = false;
	vec3 cameraPos = vec3(0., 0., 0.);
	float cameraVelocity = 0.1;

	// cam.setMouseControl(true);
	// window.grab_mouse();
	float x = 0, y = 0, z = 0;

	while (!window.should_close()) { 
		flgl::gl.clear();
		if (window.keyboard[GLFW_KEY_ESCAPE].pressed) break;
		if (window.keyboard[GLFW_KEY_K].pressed) {
			flgl::gl.wireframe(wf); wf = !wf;
		}
		if (window.keyboard[GLFW_KEY_W].down) {
			// cameraPos.z += cameraVelocity;
			z += 0.05;
		}
		if (window.keyboard[GLFW_KEY_S].down) {
			// cameraPos.z -= cameraVelocity;
			z -= 0.05;
		}
		if (window.keyboard[GLFW_KEY_A].down) {
			// cameraPos.x += cameraVelocity;
			x += 0.05;
		}
		if (window.keyboard[GLFW_KEY_D].down) {
			// cameraPos.x -= cameraVelocity;
			x -= 0.05;
		}
		if (window.keyboard[GLFW_KEY_Q].down) {
			// cameraPos.y += cameraVelocity;
			y += 0.05;
		}
		if (window.keyboard[GLFW_KEY_E].down) {
			// cameraPos.y -= cameraVelocity;
			y -= 0.05;
		}
		if (window.keyboard[GLFW_KEY_B].pressed) {
			std::cout << base_index << ": " << x << ", " << y << ", " << z << std::endl;
			base_index = (base_index + 1) % 4;
		}
		if (base_index == 0) {
			cylmat_first_base = genModelMat3d(vec3(x, y, z), vec3(0.), vec3(0.25, 0.05, 0.25));
		} else if(base_index == 1) {
			cylmat_second_base = genModelMat3d(vec3(x, y, z), vec3(0.), vec3(0.25, 0.05, 0.25));
		} else if(base_index == 2) {
			cylmat_third_base = genModelMat3d(vec3(x, y, z), vec3(0.), vec3(0.25, 0.05, 0.25));
		} else if(base_index == 3) {
			cylmat_home_base = genModelMat3d(vec3(x, y, z), vec3(0.), vec3(0.25, 0.05, 0.25));
		}
		cam.getPos() = cameraPos;
		cam.update();


		quad.bind();
		grass_shader.bind();
		grass_shader.uMat4("uProj", cam.proj());
		grass_shader.uMat4("uView", cam.view());
		grass_shader.uMat4("uModel", qmmat);

		grass.bind();

		flgl::gl.draw_elements(GL_TRIANGLES, 6, GL_UNSIGNED_INT);

		cyl_shader.bind();
		cyl_shader.uVec3("uLightDirection", glm::vec3(0.0, 1.0, 0.0)); // Change this direction as needed
		cyl_shader.uMat4("uProj", cam.proj());
		cyl_shader.uMat4("uView", cam.view());
		cyl_shader.uMat4("uModel", cylmat);


		// cyl.bind();
		// flgl::gl.draw_elements(GL_TRIANGLES, 16*6, GL_UNSIGNED_INT);

		cyl_shader.uMat4("uModel", cylmat_first_base);
		cyl.bind();
		flgl::gl.draw_elements(GL_TRIANGLES, 16*6, GL_UNSIGNED_INT);
		cyl_shader.uMat4("uModel", cylmat_second_base);
		cyl.bind();
		flgl::gl.draw_elements(GL_TRIANGLES, 16*6, GL_UNSIGNED_INT);
		cyl_shader.uMat4("uModel", cylmat_third_base);
		cyl.bind();
		flgl::gl.draw_elements(GL_TRIANGLES, 16*6, GL_UNSIGNED_INT);
		cyl_shader.uMat4("uModel", cylmat_home_base);
		cyl.bind();
		flgl::gl.draw_elements(GL_TRIANGLES, 16*6, GL_UNSIGNED_INT);

		window.update();
	}

	flgl::gl.destroy();


	return 0;
}


