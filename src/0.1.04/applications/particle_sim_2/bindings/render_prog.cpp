#include "interface_structs.h"
#include "../shader_interface.h"
#include "ui.h"
#include "resources.h"


extern GVar gvar_particle_size;
GVar        gvar_particle_rel_render_size{"Particle Render Size", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_RENDER_GENERAL, {0.01f, 5.f}};
GVar        gvar_particle_render_brightness{"Particle Render Brightness", 0.5f, GVAR_FLOAT_RANGE, GUI_CAT_RENDER_GENERAL, {0.0f, 1.f}};
GVar        gvar_particle_render_vel_brightness{"Particle Velocity Brightness", 0.5f, GVAR_FLOAT_RANGE, GUI_CAT_RENDER_GENERAL, {0.0f, 100.f}};
GVar        gvar_particle_render_force_brightness{"Particle Force Brightness", 0.5f, GVAR_FLOAT_RANGE, GUI_CAT_RENDER_GENERAL, {0.0f, 100.f}};

DrawCmd getCmdRenderParticles2D(Image target, Buffer particleBuffer, Buffer densityBuffer, Buffer forceBuffer)
{
	DrawCmd cmd = DrawCmd();
	cmd.renderArea = VkRect2D_OP(target->getExtent2D());
	addShader(cmd.pipelineDef, shaderPath + "render_particle.vert", {{"VECN_DIM", "2"}});
	addShader(cmd.pipelineDef, shaderPath + "render_particle.frag", {{"VECN_DIM", "2"}});
	cmd.pushLocal();
	cmd.pipelineDef.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

	DrawSurface ds{};
	ds.vertexBuffer = particleBuffer;
	ds.count = particleBuffer->getSize() / sizeof(GLSLParticle);
	ds.vertexLayout = vertex_type<GLSLParticle>{}.data_layout();
	cmd.setGeometry(ds);

	cmd.pushColorAttachment(target, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, BlendOperation::add(), BlendOperation::add());
	struct PushStruct
	{
		vec2  viewOffset;
		vec2  viewScale;
		vec2  extent;
		float radius;
		float intensity;
		float velIntensity;
		float forceIntensity;
	} pc;
	pc.viewOffset = vec2(0.f, 0.f);
	VkExtent2D extent = target->getExtent2D();
	float maxDim = float(max(extent.width, extent.height));
	pc.viewScale.x = maxDim / float(extent.width);
	pc.viewScale.y = maxDim / float(extent.height);
	pc.radius         = gvar_particle_rel_render_size.val.v_float * gvar_particle_size.val.v_float * cParticle_size_scale * maxDim;
	pc.intensity      = gvar_particle_render_brightness.val.v_float;
	pc.velIntensity   = gvar_particle_render_vel_brightness.val.v_float;
	pc.forceIntensity                 = gvar_particle_render_force_brightness.val.v_float;
	pc.extent = vec2(static_cast<float>(extent.width),static_cast<float>(extent.height));
	cmd.pushConstant(&pc, sizeof(PushStruct), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	cmd.pushDescriptor(densityBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	cmd.pushDescriptor(forceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	return cmd;
}

using namespace default_scene;

GVar gvar_camera_fixpoint{"Camera Fixpoint", {0.0f, 0.0f, 0.0f}, GVAR_VEC3, GUI_CAT_CAM};
GVar gvar_camera_distance{"Camera Distance", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {0.1f, 100.0f}};
GVar gvar_camera_up{"Camera Up", {0.0f, 1.0f, 0.0f}, GVAR_VEC3, GUI_CAT_CAM};
GVar gvar_camera_yaw{"Camera Yaw", 90.f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {0.0f, 360.0f}};
GVar gvar_camera_pitch{"Camera Pitch", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {-90.0f, 90.0f}};
GVar gvar_camera_move_speed{"Camera Move Speed", 2.0f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {0.1f, 10.0f}};
GVar gvar_camera_turn_speed{"Camera Turn Speed", 0.25f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {0.01f, 1.0f}};
GVar gvar_camera_scroll_speed{"Camera Scroll Speed", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_CAM, {0.01f, 1.0f}};
GVar gvar_camera_reset{"Camera Reset", false, GVAR_EVENT, GUI_CAT_CAM};

struct DefaultFixedCameraState : public vka::FixedCameraState
{
	DefaultFixedCameraState()
	{
		fixpoint    = glm::vec3(0.5f, 0.5f, 0.5f);
		distance    = 1.0;
		up          = glm::vec3(0.0f, 1.0f, 0.0f);
		yaw         = 90.f;
		pitch       = 0.0f;
		moveSpeed   = 2.0f;
		turnSpeed   = 0.25f;
		scrollSpeed = 0.1f;
	}
};

FixedCameraState loadCamState()
{
	FixedCameraState state{};
	if (gvar_camera_reset.val.v_bool)
	{
		state = DefaultFixedCameraState();
	}
	else
	{
		state.fixpoint    = gvar_camera_fixpoint.val.getVec3();
		state.distance    = gvar_camera_distance.val.v_float;
		state.up          = gvar_camera_up.val.getVec3();
		state.yaw         = gvar_camera_yaw.val.v_float;
		state.pitch       = gvar_camera_pitch.val.v_float;
		state.moveSpeed   = gvar_camera_move_speed.val.v_float;
		state.turnSpeed   = gvar_camera_turn_speed.val.v_float;
		state.scrollSpeed = gvar_camera_scroll_speed.val.v_float;
	}
	return state;
}

void saveCamState(FixedCameraState state)
{
	gvar_camera_fixpoint.val.setVec3(state.fixpoint);
	gvar_camera_distance.val.v_float = state.distance;
	gvar_camera_up.val.setVec3(state.up);
	gvar_camera_yaw.val.v_float          = state.yaw;
	gvar_camera_pitch.val.v_float        = state.pitch;
	gvar_camera_move_speed.val.v_float   = state.moveSpeed;
	gvar_camera_turn_speed.val.v_float   = state.turnSpeed;
	gvar_camera_scroll_speed.val.v_float = state.scrollSpeed;
}


DrawCmd getCmdRenderParticles3D(Image target, Buffer particleBuffer, Buffer densityBuffer, Buffer forceBuffer)
{
	DrawCmd cmd    = DrawCmd();
	Buffer  camBuf = nullptr, camInstBuf = nullptr;
	bool    fetchSuccesful = gState.dataCache->fetch(camBuf, "camBuf") && gState.dataCache->fetch(camInstBuf, "camInstBuf");
	VKA_ASSERT(fetchSuccesful);
	cmd.renderArea = VkRect2D_OP(target->getExtent2D());

	addShader(cmd.pipelineDef, shaderPath + "render_particle.vert", {{"VECN_DIM", "3"}});
	addShader(cmd.pipelineDef, shaderPath + "render_particle.frag", {{"VECN_DIM", "3"}});
	bindCamera(cmd, camBuf, camInstBuf, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	cmd.pushLocal();
	cmd.pipelineDef.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

	DrawSurface ds{};
	ds.vertexBuffer = particleBuffer;
	ds.count        = particleBuffer->getSize() / sizeof(GLSLParticle3D);
	ds.vertexLayout = vertex_type<GLSLParticle3D>{}.data_layout();
	cmd.setGeometry(ds);


	gState.depthBufferCache->fetch(target->getExtent2D());

	//cmd.pushColorAttachment(target, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, BlendOperation::add(), BlendOperation::add());
	cmd.pushColorAttachment(target, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, BlendOperation::add(), BlendOperation::add());
	cmd.pushDepthAttachment(gState.depthBufferCache->fetch(target->getExtent2D()), VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

	struct PushStruct
	{
		vec2  viewOffset;
		vec2  viewScale;
		vec2  extent;
		float radius;
		float intensity;
		float velIntensity;
		float forceIntensity;
	} pc;
	pc.viewOffset     = vec2(0.f, 0.f);
	VkExtent2D extent = target->getExtent2D();
	float      maxDim = float(max(extent.width, extent.height));
	pc.viewScale.x    = maxDim / float(extent.width);
	pc.viewScale.y    = maxDim / float(extent.height);
	pc.radius         = gvar_particle_rel_render_size.val.v_float * gvar_particle_size.val.v_float * cParticle_size_scale * maxDim;
	pc.intensity      = gvar_particle_render_brightness.val.v_float;
	pc.velIntensity   = gvar_particle_render_vel_brightness.val.v_float;
	pc.forceIntensity = gvar_particle_render_force_brightness.val.v_float;
	pc.extent         = vec2(static_cast<float>(extent.width), static_cast<float>(extent.height));
	cmd.pushConstant(&pc, sizeof(PushStruct), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	cmd.pushDescriptor(densityBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	cmd.pushDescriptor(forceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	return cmd;
}