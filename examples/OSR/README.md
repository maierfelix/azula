# OSR

This example shows how to use *azula* in [OSR mode](https://github.com/maierfelix/azula#osr-mode). *azula's* window texture get's rendered using [Vulkan](https://en.wikipedia.org/wiki/Vulkan_API).

This is an advanced example, targeted at users of [nvk](https://github.com/maierfelix/nvk) with advanded graphics programming experience.

 - A GLFW window gets created using [VulkanWindow](https://maierfelix.github.io/nvk/1.1.121/additional/VulkanWindow.html)
 - An *azula* window gets created in OSR mode
 - Input events of the GLFW window get routed to *azula's* window using [Event Dispatching](https://github.com/maierfelix/azula#event-dispatching)
 - A shared *D3D11 HANDLE* gets requested using [getSharedHandleD3D11](https://github.com/maierfelix/azula#windowprototypegetsharedhandled3d11)
 - The memory of the shared handle gets imported into Vulkan memory using [VkImportMemoryWin32HandleInfoKHR](https://maierfelix.github.io/nvk/1.1.121/structs/VkImportMemoryWin32HandleInfoKHR.html)
 - The OSR texture of *azula* then gets drawn using a simple quad shader

### Preview:

<img src="https://i.imgur.com/GCyztFx.png"/>
