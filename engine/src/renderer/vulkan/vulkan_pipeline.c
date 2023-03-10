#include "vulkan_pipeline.h"
#include "vulkan_utils.h"

#include "core/memory.h"
#include "core/log.h"

#include "math/math_types.h"

b8 vulkan_graphics_pipeline_create(
    vulkan_context* context,
    vulkan_renderpass* renderpass,
    u32 attribute_count,
    VkVertexInputAttributeDescription* attributes,
    u32 descriptor_set_layout_count,
    VkDescriptorSetLayout* descriptor_set_layouts,
    u32 stage_count,
    VkPipelineShaderStageCreateInfo* stages,
    VkViewport viewport,
    VkRect2D scissor,
    b8 is_wireframe,
    vulkan_pipeline* out_pipeline
) {
    // viewport state
    VkPipelineViewportStateCreateInfo viewport_state = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    // rasterizer state
    VkPipelineRasterizationStateCreateInfo rasterizer_state = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizer_state.depthClampEnable = VK_FALSE;
    rasterizer_state.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_state.polygonMode =
        is_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizer_state.lineWidth = 1.0f;
    rasterizer_state.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_state.depthBiasEnable = VK_FALSE;
    rasterizer_state.depthBiasConstantFactor = 0.0f;
    rasterizer_state.depthBiasClamp = 0.0f;
    rasterizer_state.depthBiasSlopeFactor = 0.0f;

    // multisample state
    VkPipelineMultisampleStateCreateInfo multisample_state = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisample_state.sampleShadingEnable = VK_FALSE;
    multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state.minSampleShading = 1.0f;
    multisample_state.pSampleMask = 0;
    multisample_state.alphaToCoverageEnable = VK_FALSE;
    multisample_state.alphaToOneEnable = VK_FALSE;

    // depth stencil state
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depth_stencil_state.depthTestEnable = VK_TRUE;
    depth_stencil_state.depthWriteEnable = VK_TRUE;
    depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state.stencilTestEnable = VK_FALSE;

    // color blend state
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    memory_zero(&color_blend_attachment, sizeof(color_blend_attachment));
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_state = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    color_blend_state.logicOpEnable = VK_FALSE;
    color_blend_state.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &color_blend_attachment;

    // dynamic state
    const u32 dynamic_state_count = 3;
    VkDynamicState dynamic_states[dynamic_state_count] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamic_state = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic_state.dynamicStateCount = dynamic_state_count;
    dynamic_state.pDynamicStates = dynamic_states;

    // vertex input state
    VkVertexInputBindingDescription binding_description;
    binding_description.binding = 0;
    binding_description.stride = sizeof(vertex_3d);
    binding_description.inputRate =
        VK_VERTEX_INPUT_RATE_VERTEX;  // move to next data entry after each
                                      // vertex

    VkPipelineVertexInputStateCreateInfo vertex_input_state = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertex_input_state.vertexBindingDescriptionCount = 1;
    vertex_input_state.pVertexBindingDescriptions = &binding_description;
    vertex_input_state.vertexAttributeDescriptionCount = attribute_count;
    vertex_input_state.pVertexAttributeDescriptions = attributes;

    // input assembly state
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state.primitiveRestartEnable = VK_FALSE;

    // pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    // push constants
    VkPushConstantRange push_constant;
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant.offset = sizeof(mat4) * 0;
    push_constant.size = sizeof(mat4) * 2;

    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant;

    // descriptor set layout
    pipeline_layout_info.setLayoutCount = descriptor_set_layout_count;
    pipeline_layout_info.pSetLayouts = descriptor_set_layouts;

    // create pipeline layout
    VK_CHECK(vkCreatePipelineLayout(
        context->device.logical_device,
        &pipeline_layout_info,
        context->allocator,
        &out_pipeline->pipeline_layout
    ));

    // pipeline create
    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipeline_create_info.stageCount = stage_count;
    pipeline_create_info.pStages = stages;
    pipeline_create_info.pVertexInputState = &vertex_input_state;
    pipeline_create_info.pInputAssemblyState = &input_assembly_state;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pRasterizationState = &rasterizer_state;
    pipeline_create_info.pMultisampleState = &multisample_state;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state;
    pipeline_create_info.pColorBlendState = &color_blend_state;
    pipeline_create_info.pDynamicState = &dynamic_state;
    pipeline_create_info.pTessellationState = 0;
    pipeline_create_info.layout = out_pipeline->pipeline_layout;
    pipeline_create_info.renderPass = renderpass->handle;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;

    VkResult result = vkCreateGraphicsPipelines(
        context->device.logical_device,
        VK_NULL_HANDLE,
        1,
        &pipeline_create_info,
        context->allocator,
        &out_pipeline->handle
    );

    if (vulkan_result_is_success(result)) {
        OKO_INFO("Graphics pipeline created!");
        return true;
    }

    OKO_ERROR(
        "Failed to create graphics pipeline! %s",
        vulkan_result_string(result, true)
    );
    return false;
}

void vulkan_pipeline_destroy(
    vulkan_context* context, vulkan_pipeline* pipeline
) {
    if (pipeline) {
        if (pipeline->handle) {
            vkDestroyPipeline(
                context->device.logical_device,
                pipeline->handle,
                context->allocator
            );
        }
        if (pipeline->pipeline_layout) {
            vkDestroyPipelineLayout(
                context->device.logical_device,
                pipeline->pipeline_layout,
                context->allocator
            );
        }
    }
}

void vulkan_pipeline_bind(
    vulkan_command_buffer* command_buffer,
    VkPipelineBindPoint bind_point,
    vulkan_pipeline* pipeline
) {
    vkCmdBindPipeline(command_buffer->handle, bind_point, pipeline->handle);
}