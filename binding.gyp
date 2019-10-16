{
  "variables": {
    "root": "./",
    "platform": "<(OS)",
    "release": "<@(module_root_dir)/build/Release",
    "vkSDK": "C:/VulkanSDK/1.1.121.2"
  },
  "conditions": [
    [ "platform == 'win'", { "variables": { "platform": "win32" } } ]
  ],
  "targets": [
    {
      "target_name": "action_after_build",
      "type": "none",
      "conditions": []
    },
    {
      "sources": [
        "./src/GUIFrame.cpp",
        "./src/GUIRenderer.cpp",
        "./src/GUIRendererD3D11.cpp",
        "./src/FileSystemWin.cpp",
        "./src/FontLoaderWin.cpp",
        "./src/GPUDriverD3D11.cpp",
        "./src/GPUContextD3D11.cpp",
        "./src/index.cpp",
      ],
      "conditions": [
        [
          "OS=='win'",
          {
            "target_name": "addon-win32",
            "cflags": [
              "-stdlib=libstdc++",
            ],
            "include_dirs": [
              "<!@(node -p \"require('node-addon-api').include\")",
              "<(root)/third_party/include/",
              "<(vkSDK)/include",
            ],
            "library_dirs": [
              "<(root)/third_party/lib/<(platform)/<(target_arch)/*",
              "<(vkSDK)/lib",
            ],
            "link_settings": {
              "libraries": [
                "D3DCompiler.lib",
                "d3d11.lib",
                #"glfw3dll.lib",
                "vulkan-1.lib",
                "AppCore.lib",
                "Ultralight.lib",
                "UltralightCore.lib",
                "WebCore.lib",
                "usp10.lib",
                "Shlwapi.lib",
              ]
            },
            "defines": [
              "WIN32_LEAN_AND_MEAN",
              "VC_EXTRALEAN",
              "_HAS_EXCEPTIONS=1",
              "NOMINMAX"
            ],
            "msvs_configuration_attributes": {
              "CharacterSet": "Unicode"
            },
            "msvs_settings": {
              "VCCLCompilerTool": {
                "AdditionalOptions": [
                  "/MP /EHsc"
                ],
                "ExceptionHandling": 1
              },
              "VCLibrarianTool": {
                "AdditionalOptions" : [
                  "/NODEFAULTLIB:MSVCRT",
                ]
              },
              "VCLinkerTool": {
                "AdditionalLibraryDirectories": [
                  "@PROJECT_SOURCE_DIR@/third_party/lib/<(platform)/<(target_arch)",
                ]
              }
            }
          }
        ]
      ]
    }
  ]
}
