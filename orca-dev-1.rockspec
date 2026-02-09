package = "orca"
version = "dev-1"

source = {
  url = "git://github.com/corepunch/orca.git",
  branch = "main"
}

description = {
  summary = "ORCA - A modern UI framework for Lua with MoonScript support",
  detailed = [[
    ORCA is a comprehensive UI framework that provides:
    - Native UI components (buttons, grids, stacks, text, images)
    - 2D and 3D scene management (UIKit and SceneKit)
    - XML/CSS/JSON parsing
    - Networking and filesystem support
    - Hot reloading for rapid development
    - MoonScript support for elegant syntax
    - Localization/i18n
    Perfect for building desktop applications, instrument clusters, and interactive UIs.
  ]],
  homepage = "https://github.com/corepunch/orca",
  license = "MIT",
  maintainer = "Igor Chernakov <igor.chernakov@gmail.com>"
}

dependencies = {
  "lua >= 5.4, < 5.5"
}

external_dependencies = {
  LIBPNG = {
    header = "png.h",
    library = "png"
  },
  FREETYPE = {
    header = "ft2build.h",
    library = "freetype"
  },
  JPEG = {
    header = "jpeglib.h",
    library = "jpeg"
  },
  XML2 = {
    header = "libxml/tree.h",
    library = "xml2"
  },
  LZ4 = {
    header = "lz4.h",
    library = "lz4"
  },
  ZLIB = {
    header = "zlib.h",
    library = "z"
  },
  CURL = {
    header = "curl/curl.h",
    library = "curl"
  }
}

build = {
  type = "make",
  build_variables = {
    CFLAGS = "$(CFLAGS) -I$(LUA_INCDIR) -fPIC",
    LIBFLAG = "$(LIBFLAG)",
    LUA_LIBDIR = "$(LUA_LIBDIR)",
    LUA_BINDIR = "$(LUA_BINDIR)",
    LUA_INCDIR = "$(LUA_INCDIR)",
    LUA = "$(LUA)",
  },
  install_variables = {
    INST_PREFIX = "$(PREFIX)",
    INST_BINDIR = "$(BINDIR)",
    INST_LIBDIR = "$(LIBDIR)",
    INST_LUADIR = "$(LUADIR)",
  },
  platforms = {
    unix = {
      build_target = "all",
      install_target = "install"
    },
    macosx = {
      build_target = "all",
      install_target = "install"
    }
  }
}
