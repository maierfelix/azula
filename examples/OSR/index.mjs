import fs from "fs";
import nvk from "nvk";
import azula from "azula";
import essentials from "nvk-essentials";

const {GLSL} = essentials;

Object.assign(global, nvk);

function ASSERT_VK_RESULT(result) {
  if (result !== VK_SUCCESS) {
    for (let key in VkResult) {
      if (VkResult[key] === result) {
        throw new Error(`Vulkan assertion failed: '${key}'`);
      }
    };
    throw new Error(`Vulkan assertion failed: '${result}'`);
  }
};

function getMemoryTypeIndex(typeFilter, propertyFlag) {
  let memoryProperties = new VkPhysicalDeviceMemoryProperties();
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, memoryProperties);
  for (let ii = 0; ii < memoryProperties.memoryTypeCount; ++ii) {
    if (
      (typeFilter & (1 << ii)) &&
      (memoryProperties.memoryTypes[ii].propertyFlags & propertyFlag) === propertyFlag
    ) {
      return ii;
    }
  };
  return -1;
};

let result = VK_SUCCESS;

let win = new VulkanWindow({
  width: 512,
  height: 512,
  title: "NVK GUI",
  resizable: false
});

let gui = new azula.Window({
  width: win.width,
  height: win.height,
  useOffscreenRendering: true
});

let handleWin32 = gui.getSharedHandleD3D11();

// forward window mouse events to GUI
let currentButton = -1;
win.onmousemove = e => {
  gui.dispatchMouseEvent("onmousemove", e.x, e.y, currentButton);
};
win.onmousedown = e => {
  currentButton = e.button;
  gui.dispatchMouseEvent("onmousedown", e.x, e.y, e.button);
};
win.onmouseup = e => {
  currentButton = -1;
  gui.dispatchMouseEvent("onmouseup", e.x, e.y, e.button);
};
win.onmousewheel = e => {
  gui.dispatchScrollEvent("onmousewheel", e.deltaX, e.deltaY);
};

// forward window key events to GUI
win.onkeydown = e => {
  gui.dispatchKeyEvent("onkeydown", e.keyCode);
};
win.onkeyup = e => {
  gui.dispatchKeyEvent("onkeyup", e.keyCode);
};

// forward GUI cursor change event to window
gui.oncursorchange = e => {
  //console.log("Cursor:", e);
};

// forwards GUI console calls to node CLI
gui.onconsolemessage = e => {
  let message = e.message;
  let loc = `at ${e.source ? e.source + ":" : ""}${e.location.line}:${e.location.column}`;
  e.callee.apply(console, [e.message, loc]);
};

gui.onbinarymessage = (buffer, args) => {
  setTimeout(() => {
    args.kind = 666;
    console.log(buffer);
    gui.dispatchBinaryBuffer(buffer, args);
  }, 1e3);
};

gui.loadHTML(fs.readFileSync("./index.html", "utf8"));
gui.onbinarymessage(new ArrayBuffer(16), { kind: 420 });

let device = new VkDevice();
let instance = new VkInstance();

let queue = new VkQueue();
let commandPool = new VkCommandPool();

let pipeline = new VkPipeline();
let pipelineLayout = new VkPipelineLayout();

let descriptorSet = new VkDescriptorSet();
let descriptorPool = new VkDescriptorPool();
let descriptorSetLayout = new VkDescriptorSetLayout();

let vertShaderModule = new VkShaderModule();
let fragShaderModule = new VkShaderModule();

let surface = new VkSurfaceKHR();
let renderPass = new VkRenderPass();
let swapchain = new VkSwapchainKHR();

let guiImage = new VkImage();
let guiImageView = new VkImageView();
let guiImageMemory = new VkDeviceMemory();
let guiImageSampler = new VkSampler();

let semaphoreImageAvailable = new VkSemaphore();
let semaphoreRenderingAvailable = new VkSemaphore();

let physicalDevice = null;

let imageViews = [];
let framebuffers = [];
let commandBuffers = [];

let amountOfImagesInSwapchain = { $: 0 };

let validationLayers = [
  "VK_LAYER_LUNARG_standard_validation"
];

let deviceExtensions = [
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
];

let enabledFeatures = new VkPhysicalDeviceFeatures();
enabledFeatures.samplerAnisotropy = true;

let instanceExtensions = win.getRequiredInstanceExtensions();

/** Create Instance **/
{
  let appInfo = new VkApplicationInfo();
  appInfo.pApplicationName = "Hello!";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_1;

  let createInfo = new VkInstanceCreateInfo();
  createInfo.pApplicationInfo = appInfo;
  createInfo.enabledExtensionCount = instanceExtensions.length;
  createInfo.ppEnabledExtensionNames = instanceExtensions;
  createInfo.enabledLayerCount = 0;
  createInfo.enabledLayerCount = validationLayers.length;
  createInfo.ppEnabledLayerNames = validationLayers;

  result = vkCreateInstance(createInfo, null, instance);
  ASSERT_VK_RESULT(result);
}

/** Create Physical Device **/
{
  let deviceCount = { $:0 };
  vkEnumeratePhysicalDevices(instance, deviceCount, null);
  if (deviceCount.$ <= 0) console.error("Error: No render devices available!");
  let devices = [...Array(deviceCount.$)].map(() => new VkPhysicalDevice());
  result = vkEnumeratePhysicalDevices(instance, deviceCount, devices);
  ASSERT_VK_RESULT(result);

  physicalDevice = devices[0];

  let deviceProperties = new VkPhysicalDeviceProperties();
  vkGetPhysicalDeviceProperties(physicalDevice, deviceProperties);

  console.log(`Using device: ${deviceProperties.deviceName}`);
}

/** Create Logical Device **/
{
  let deviceQueueInfo = new VkDeviceQueueCreateInfo();
  deviceQueueInfo.queueFamilyIndex = 0;
  deviceQueueInfo.queueCount = 1;
  deviceQueueInfo.pQueuePriorities = new Float32Array([1.0, 1.0, 1.0, 1.0]);

  let deviceInfo = new VkDeviceCreateInfo();
  deviceInfo.queueCreateInfoCount = 1;
  deviceInfo.pQueueCreateInfos = [deviceQueueInfo];
  deviceInfo.enabledExtensionCount = deviceExtensions.length;
  deviceInfo.ppEnabledExtensionNames = deviceExtensions;
  deviceInfo.pEnabledFeatures = enabledFeatures;

  result = vkCreateDevice(physicalDevice, deviceInfo, null, device);
  ASSERT_VK_RESULT(result);
}

/** Create Device Queue **/
{
  vkGetDeviceQueue(device, 0, 0, queue);
}

/** Create Command Pool **/
{
  let commandPoolInfo = new VkCommandPoolCreateInfo();
  commandPoolInfo.queueFamilyIndex = 0;

  result = vkCreateCommandPool(device, commandPoolInfo, null, commandPool);
  ASSERT_VK_RESULT(result);
}

/** Create GUI Image **/
{
  let descImageCreateInfo = new VkImageCreateInfo();
  descImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  descImageCreateInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
  descImageCreateInfo.extent.width = win.width;
  descImageCreateInfo.extent.height = win.height;
  descImageCreateInfo.extent.depth = 1;
  descImageCreateInfo.mipLevels = 1;
  descImageCreateInfo.arrayLayers = 1;
  descImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  descImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  descImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  descImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  descImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  result = vkCreateImage(device, descImageCreateInfo, null, guiImage);
  ASSERT_VK_RESULT(result);

  let memoryRequirements = new VkMemoryRequirements();
  vkGetImageMemoryRequirements(device, guiImage, memoryRequirements);

  let importMemoryWin32HandleInfoKHRInfo = new VkImportMemoryWin32HandleInfoKHR();
  importMemoryWin32HandleInfoKHRInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
  importMemoryWin32HandleInfoKHRInfo.handle = handleWin32;

  let descMemoryAllocateInfo = new VkMemoryAllocateInfo();
  descMemoryAllocateInfo.pNext = importMemoryWin32HandleInfoKHRInfo;
  descMemoryAllocateInfo.allocationSize = memoryRequirements.size;
  descMemoryAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(
    memoryRequirements.memoryTypeBits,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );

  result = vkAllocateMemory(device, descMemoryAllocateInfo, null, guiImageMemory);
  ASSERT_VK_RESULT(result);

  result = vkBindImageMemory(device, guiImage, guiImageMemory, 0n);
  ASSERT_VK_RESULT(result);

  let imageViewInfo = new VkImageViewCreateInfo();
  imageViewInfo.image = guiImage;
  imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
  imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
  imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
  imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
  imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
  imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageViewInfo.subresourceRange.levelCount = 1;
  imageViewInfo.subresourceRange.layerCount = 1;
  result = vkCreateImageView(device, imageViewInfo, null, guiImageView);
  ASSERT_VK_RESULT(result);

  let samplerInfo = new VkSamplerCreateInfo();
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.anisotropyEnable = true;
  samplerInfo.maxAnisotropy = 16.0;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = false;
  samplerInfo.compareEnable = false;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  result = vkCreateSampler(device, samplerInfo, null, guiImageSampler);
  ASSERT_VK_RESULT(result);

  // transition the image layout from undefined -> shader read-only optimal
  {
    let oldImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    let newImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    let commandBufferAllocateInfo = new VkCommandBufferAllocateInfo();
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;

    let commandBuffer = new VkCommandBuffer();
    vkAllocateCommandBuffers(device, commandBufferAllocateInfo, [commandBuffer]);

    let commandBufferBeginInfo = new VkCommandBufferBeginInfo();
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, commandBufferBeginInfo);

    let barrier = new VkImageMemoryBarrier();
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = guiImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    let sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    let destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
      commandBuffer,
      sourceStage, destinationStage,
      0,
      0, null,
      0, null,
      1, [barrier]
    );

    vkEndCommandBuffer(commandBuffer);

    let submitInfo = new VkSubmitInfo();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = [commandBuffer];

    vkQueueSubmit(queue, 1, [submitInfo], null);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, [commandBuffer]);
  }
}

/** Create descriptors **/
{
  let descriptorSetLayoutBinding = new VkDescriptorSetLayoutBinding();
  descriptorSetLayoutBinding.binding = 0;
  descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorSetLayoutBinding.descriptorCount = 1;
  descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  descriptorSetLayoutBinding.pImmutableSamplers = null;

  let descriptorSetLayoutInfo = new VkDescriptorSetLayoutCreateInfo();
  descriptorSetLayoutInfo.bindingCount = 1;
  descriptorSetLayoutInfo.pBindings = [descriptorSetLayoutBinding];

  result = vkCreateDescriptorSetLayout(device, descriptorSetLayoutInfo, null, descriptorSetLayout);
  ASSERT_VK_RESULT(result);

  let descriptorPoolInfo = new VkDescriptorPoolCreateInfo();
  descriptorPoolInfo.maxSets = 1;
  descriptorPoolInfo.poolSizeCount = 1;
  descriptorPoolInfo.pPoolSizes = [
    new VkDescriptorPoolSize({
      type: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      descriptorCount: 1
    })
  ];

  result = vkCreateDescriptorPool(device, descriptorPoolInfo, null, descriptorPool);
  ASSERT_VK_RESULT(result);

  let descriptorSetAllocInfo = new VkDescriptorSetAllocateInfo();
  descriptorSetAllocInfo.descriptorPool = descriptorPool;
  descriptorSetAllocInfo.descriptorSetCount = 1;
  descriptorSetAllocInfo.pSetLayouts = [descriptorSetLayout];
  result = vkAllocateDescriptorSets(device, descriptorSetAllocInfo, [descriptorSet]);
  ASSERT_VK_RESULT(result);

  let descriptorImageInfo = new VkDescriptorImageInfo();
  descriptorImageInfo.sampler = guiImageSampler;
  descriptorImageInfo.imageView = guiImageView;
  descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  let writeDescriptorSet = new VkWriteDescriptorSet();
  writeDescriptorSet.dstSet = descriptorSet;
  writeDescriptorSet.dstBinding = 0;
  writeDescriptorSet.dstArrayElement = 0;
  writeDescriptorSet.descriptorCount = 1;
  writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writeDescriptorSet.pImageInfo = [descriptorImageInfo];
  writeDescriptorSet.pBufferInfo = null;
  writeDescriptorSet.pTexelBufferView = null;

  vkUpdateDescriptorSets(device, 1, [writeDescriptorSet], 0, null);
}

/** Create Surface **/
{
  result = win.createSurface(instance, null, surface);
  ASSERT_VK_RESULT(result);

  let surfaceSupport = { $: false };
  vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, surfaceSupport);
  if (!surfaceSupport) console.error(`No surface creation support!`);
}

/** Create Swapchain **/
{
  let swapchainInfo = new VkSwapchainCreateInfoKHR();
  swapchainInfo.surface = surface;
  swapchainInfo.minImageCount = 3;
  swapchainInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
  swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  swapchainInfo.imageExtent.width = win.width;
  swapchainInfo.imageExtent.height = win.height
  swapchainInfo.imageArrayLayers = 1;
  swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainInfo.queueFamilyIndexCount = 0;
  swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  swapchainInfo.clipped = true;
  swapchainInfo.oldSwapchain = null;

  result = vkCreateSwapchainKHR(device, swapchainInfo, null, swapchain);
  ASSERT_VK_RESULT(result);

  vkGetSwapchainImagesKHR(device, swapchain, amountOfImagesInSwapchain, null);
  let swapchainImages = [...Array(amountOfImagesInSwapchain.$)].map(() => new VkImage());

  result = vkGetSwapchainImagesKHR(device, swapchain, amountOfImagesInSwapchain, swapchainImages);
  ASSERT_VK_RESULT(result);

  for (let ii = 0; ii < amountOfImagesInSwapchain.$; ++ii) {
    let imageViewInfo = new VkImageViewCreateInfo();
    imageViewInfo.image = swapchainImages[ii];
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    imageViews[ii] = new VkImageView();
    result = vkCreateImageView(device, imageViewInfo, null, imageViews[ii])
    ASSERT_VK_RESULT(result);
  };
}

/** Create Shader Modules **/
{
  let vertSrc = GLSL.toSPIRVSync({
    source: fs.readFileSync(`./shaders/gui.vert`),
    extension: `vert`
  }).output;

  let fragSrc = GLSL.toSPIRVSync({
    source: fs.readFileSync(`./shaders/gui.frag`),
    extension: `frag`
  }).output;

  let vertShaderModuleInfo = new VkShaderModuleCreateInfo();
  vertShaderModuleInfo.pCode = vertSrc;
  vertShaderModuleInfo.codeSize = vertSrc.byteLength;
  result = vkCreateShaderModule(device, vertShaderModuleInfo, null, vertShaderModule);
  ASSERT_VK_RESULT(result);

  let fragShaderModuleInfo = new VkShaderModuleCreateInfo();
  fragShaderModuleInfo.pCode = fragSrc;
  fragShaderModuleInfo.codeSize = fragSrc.byteLength;
  result = vkCreateShaderModule(device, fragShaderModuleInfo, null, fragShaderModule);
  ASSERT_VK_RESULT(result);
}

/** Create Pipeline Layout **/
{
  let pipelineLayoutInfo = new VkPipelineLayoutCreateInfo();
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = [descriptorSetLayout];

  result = vkCreatePipelineLayout(device, pipelineLayoutInfo, null, pipelineLayout);
  ASSERT_VK_RESULT(result);
}

/** Create Render Pass **/
{
  let attachmentDescription = new VkAttachmentDescription();
  attachmentDescription.flags = 0;
  attachmentDescription.format = VK_FORMAT_B8G8R8A8_SRGB;
  attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
  attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  let attachmentReference = new VkAttachmentReference();
  attachmentReference.attachment = 0;
  attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  let subpassDescription = new VkSubpassDescription();
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = [attachmentReference];

  let subpassDependency = new VkSubpassDependency();
  subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstAccessMask = (
    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
  );
  subpassDependency.dependencyFlags = 0;

  let renderPassInfo = new VkRenderPassCreateInfo();
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = [attachmentDescription];
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = [subpassDescription];
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = [subpassDependency];

  result = vkCreateRenderPass(device, renderPassInfo, null, renderPass);
  ASSERT_VK_RESULT(result);
}

/** Create Graphics Pipeline **/
{
  let vertShaderStageInfo = new VkPipelineShaderStageCreateInfo();
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  let fragShaderStageInfo = new VkPipelineShaderStageCreateInfo();
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  let viewport = new VkViewport();
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = win.width;
  viewport.height = win.height;
  viewport.minDepth = 0.0;
  viewport.maxDepth = 1.0;

  let scissor = new VkRect2D();
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = win.width;
  scissor.extent.height = win.height;

  let viewportStateInfo = new VkPipelineViewportStateCreateInfo();
  viewportStateInfo.viewportCount = 1;
  viewportStateInfo.pViewports = [viewport];
  viewportStateInfo.scissorCount = 1;
  viewportStateInfo.pScissors = [scissor];

  let rasterizationInfo = new VkPipelineRasterizationStateCreateInfo();
  rasterizationInfo.depthClampEnable = false;
  rasterizationInfo.rasterizerDiscardEnable = false;
  rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
  rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationInfo.depthBiasEnable = false;
  rasterizationInfo.depthBiasConstantFactor = 0.0;
  rasterizationInfo.depthBiasClamp = 0.0;
  rasterizationInfo.depthBiasSlopeFactor = 0.0;
  rasterizationInfo.lineWidth = 1.0;

  let multisampleInfo = new VkPipelineMultisampleStateCreateInfo();
  multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleInfo.minSampleShading = 1.0;
  multisampleInfo.pSampleMask = null;
  multisampleInfo.alphaToCoverageEnable = false;
  multisampleInfo.alphaToOneEnable = false;

  let colorBlendAttachment = new VkPipelineColorBlendAttachmentState();
  colorBlendAttachment.blendEnable = true;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.colorWriteMask = (
    VK_COLOR_COMPONENT_R_BIT |
    VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT |
    VK_COLOR_COMPONENT_A_BIT
  );

  let colorBlendInfo = new VkPipelineColorBlendStateCreateInfo();
  colorBlendInfo.logicOpEnable = false;
  colorBlendInfo.logicOp = VK_LOGIC_OP_NO_OP;
  colorBlendInfo.attachmentCount = 1;
  colorBlendInfo.pAttachments = [colorBlendAttachment];
  colorBlendInfo.blendConstants = [0.0, 0.0, 0.0, 0.0];

  let graphicsPipelineInfo = new VkGraphicsPipelineCreateInfo();
  graphicsPipelineInfo.stageCount = 2;
  graphicsPipelineInfo.pStages = [vertShaderStageInfo, fragShaderStageInfo];
  graphicsPipelineInfo.pVertexInputState = new VkPipelineVertexInputStateCreateInfo();
  graphicsPipelineInfo.pInputAssemblyState = new VkPipelineInputAssemblyStateCreateInfo({
    topology: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
  });
  graphicsPipelineInfo.pViewportState = viewportStateInfo;
  graphicsPipelineInfo.pRasterizationState = rasterizationInfo;
  graphicsPipelineInfo.pMultisampleState = multisampleInfo;
  graphicsPipelineInfo.pColorBlendState = colorBlendInfo;
  graphicsPipelineInfo.layout = pipelineLayout;
  graphicsPipelineInfo.renderPass = renderPass;
  graphicsPipelineInfo.subpass = 0;
  graphicsPipelineInfo.basePipelineIndex = -1;

  result = vkCreateGraphicsPipelines(device, null, 1, [graphicsPipelineInfo], null, [pipeline]);
  ASSERT_VK_RESULT(result);

  for (let ii = 0; ii < amountOfImagesInSwapchain.$; ++ii) {
    let framebufferInfo = new VkFramebufferCreateInfo();
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = [imageViews[ii]];
    framebufferInfo.width = win.width;
    framebufferInfo.height = win.height;
    framebufferInfo.layers = 1;
    framebuffers[ii] = new VkFramebuffer();
    result = vkCreateFramebuffer(device, framebufferInfo, null, framebuffers[ii]);
    ASSERT_VK_RESULT(result);
  };
}

/** Create Command Buffers **/
{
  let commandBufferAllocateInfo = new VkCommandBufferAllocateInfo();
  commandBufferAllocateInfo.commandPool = commandPool;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = amountOfImagesInSwapchain.$;

  for (let ii = 0; ii < amountOfImagesInSwapchain.$; ++ii) {
    commandBuffers.push(new VkCommandBuffer());
  };

  result = vkAllocateCommandBuffers(device, commandBufferAllocateInfo, commandBuffers);
  ASSERT_VK_RESULT(result);

  /** Record Command Buffers **/
  for (let ii = 0; ii < commandBuffers.length; ++ii) {
    let commandBuffer = commandBuffers[ii];

    let commandBufferBeginInfo = new VkCommandBufferBeginInfo();
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    commandBufferBeginInfo.pInheritanceInfo = null;
    result = vkBeginCommandBuffer(commandBuffer, commandBufferBeginInfo);
    ASSERT_VK_RESULT(result);

    let renderPassBeginInfo = new VkRenderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = framebuffers[ii];
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = win.width;
    renderPassBeginInfo.renderArea.extent.height = win.height;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = [new VkClearValue()];

    vkCmdBeginRenderPass(commandBuffer, renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, [descriptorSet], 0, null);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    result = vkEndCommandBuffer(commandBuffer);
    ASSERT_VK_RESULT(result);
  };
}

/** Create Synchronization **/
{
  let semaphoreInfo = new VkSemaphoreCreateInfo();
  result = vkCreateSemaphore(device, semaphoreInfo, null, semaphoreImageAvailable);
  ASSERT_VK_RESULT(result);
  result = vkCreateSemaphore(device, semaphoreInfo, null, semaphoreRenderingAvailable);
  ASSERT_VK_RESULT(result);
}

function drawFrame() {
  let imageIndex = { $: 0 };
  vkAcquireNextImageKHR(device, swapchain, Number.MAX_SAFE_INTEGER, semaphoreImageAvailable, null, imageIndex);

  let submitInfo = new VkSubmitInfo();
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = [semaphoreImageAvailable];
  submitInfo.pWaitDstStageMask = new Int32Array([
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  ]);
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = [commandBuffers[imageIndex.$]];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = [semaphoreRenderingAvailable];

  result = vkQueueSubmit(queue, 1, [submitInfo], null);
  ASSERT_VK_RESULT(result);

  let presentInfo = new VkPresentInfoKHR();
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = [semaphoreRenderingAvailable];
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = [swapchain];
  presentInfo.pImageIndices = new Uint32Array([imageIndex.$]);
  presentInfo.pResults = null;

  return vkQueuePresentKHR(queue, presentInfo);
};

(function drawLoop() {
  win.pollEvents();
  let result = drawFrame();
  gui.update();
  gui.flush();
  if (result === VK_SUBOPTIMAL_KHR || result === VK_ERROR_OUT_OF_DATE_KHR) {
    // end
  } else {
    ASSERT_VK_RESULT(result);
    setImmediate(drawLoop);
  }
})();
