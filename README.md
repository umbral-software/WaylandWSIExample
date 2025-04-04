![Build](https://github.com/umbral-software/WaylandWSIExample/actions/workflows/cmake-single-platform.yml/badge.svg)

# Wayland-Vulkan WSI Example

An example Wayland backend for Vulkan projects.

## Description

A number of developers have experienced issues writing a custom Wayland backend for their Vulkan projects. Between poor documentation (e.g wl_frobble_bar: Frobbles a Bar) and poor examples (e.g. still using wl_shell).

This project aims to be a minimal but complete wayland client for use with vulkan, and easily extensible to other APIs such as OpenGL. It uses the core protocol and xdg shell to handle window setup and management, keyboard, mouse and touch input and basic cursor handling.
Where supported, it also will suppport server-side decorations (failing back to fullscreen-only on compositors that do not provide the protocol), setting a content type hint for the compositor, and using the modern cursor shape protocol.

Full list of protocols supported:
* [Core protocol](https://wayland.app/protocols/wayland)
* [XDG shell](https://wayland.app/protocols/xdg-shell) (required)
* [Content type hint](https://wayland.app/protocols/content-type-v1) (optional)
* [Cursor Shape](https://wayland.app/protocols/cursor-shape-v1) (optional)
* [Fractional Scale](https://wayland.app/protocols/fractional-scale-v1) (optional)
* [Viewporter](https://wayland.app/protocols/viewporter) (optional, required for fractional scale)
* [XDG Decoration](https://wayland.app/protocols/xdg-decoration-unstable-v1) (optional, mandatory for non-fullscreen windows)

Also required to build, but not used:
* [Tablet v2](https://wayland.app/protocols/tablet-v2) (build dependency of Cursor Shape protocol)

## Known Issues

* No client side decoration support, only fullscreen is suppported if XDG Decoration is not provided by the compositor. This is considered WONTFIX, developers should consider implementing libdecor if they need client side decorations, but this is incompatible with the raw use of xdg_shell protocols used by this project.

## Dependencies

* C/C++ compiler (tested with clang++ 18)
* CMake
* [Extra Cmake Modules](https://api.kde.org/frameworks/extra-cmake-modules/html/index.html) by KDE
* GLM headers
* pkg-config executable
* Vulkan SDK
  * Vulkan headers
  * VMA headers
  * Volk header
  * Utility headers
  * glslc executable
* Wayland Client headers/library
* Wayland Cursor headers/library
* Wayland Protocols library
  * stable/xdg-shell
  * stable/viewporter
  * staging/content-type
  * staging/cursor-shape
  * staging/fractional-scale
  * unstable/tablet v2
  * unstable/xdg-decoration
* Wayland Scanner executable
* XKBCommon headers/library

## Testing

Currently the code is tested on KWin, Mutter, Weston and Sway. Reports (both sucessful or bugs) from people using other compositors are welcome.

## Contributing

* Bugs: Please create an issue or contact me on Discord.
* New features: Pull requests or issues welcome.

## Authors

* Karen Webb
  * Discord: KarenTheDorf(hildarthedorf)
  * Email: [karen.webb@umbralsoftware.co.uk](mailto:karen.webb@umbralsoftware.co.uk)
  * IRC: ##umbral on [Libera](https://libera.chat/)

## Version History

* 0.1
    * Initial Release

## License

This project is licensed under the MIT License - see the LICENSE.md file for details

## Acknowledgments

* Members of the [Vulkan Discord](https://www.discord.gg/vulkan) for driving me insane with their questions to the point I wrote this project
* #wayland on [OFTC](https://www.oftc.net/) IRC for their help in understanding the protocol
