local core = require "orca.core"
local core2 = require "orca.core2"
local filesystem = require "orca.filesystem"

local libraries = {
  AnimationClipLibrary = "Animation Clips",
  ScreenLibrary = "Screens",
  MaterialTypeLibrary = "Shaders",
  MaterialLibrary = "Materials",
  BrushLibrary = "Brushes",
  MeshLibrary = "Mesh Data",
  TimelineSequenceLibrary = "Timeline Sequences",
  SceneObjectLibrary = "Objects",
  ComposerLibrary = "Render Passes",
  SceneLibrary = "Scenes",
  TrajectoryLibrary = "Trajectories",
  TransitionLibrary = "Transitions",
  SplineLibrary = "Splines",
  PrefabLibrary = "Prefabs",
  ProfileLibrary = "Profiles",
  EnginePluginLibrary = "Engine Plugins",
  ShortcutLibrary = "Shortcuts",
  LayerLibrary = "Layers",
  AnimationLibrary = "Animations",
  TagLibrary = "Tags",
  ThemeLibrary = "Themes",
  ResourceExportTagLibrary = "Resource Export Tags",
  LocaleLibrary = "Locales",
  DataSourceLibrary = "Data Sources",
  PageTransitionCollectionLibrary = "Page Transition Collections",
  ResourceFilesItem = "Resource Files",
  TextureLibrary = "Textures",
  StyleLibrary = "Styles",
  StateManagerLibrary = "State Managers",
  ConnectServiceLibrary = "Connect Services",
  ConnectUserServiceLibrary = "Connect User Services",
	ImageLibrary = "Images",
  SpriteLibrary = "Sprites",
  SpriteAnimationLibrary = "Sprite Animations",
	SystemMessageLibrary = "System Messages",
	PipelineItemLibrary = "Pipeline Items",
	ComponentTypeLibrary = "Component Types",
	DataSourceTypeLibrary = "Data Source Types",
	RenderPassTypeLibrary = "Render Pass Types",
	NodeComponentTypeLibrary = "Node Component Types",
	TriggerActionTypeLibrary = "Trigger Action Types",
	MessageTypeLibrary = "Message Types",
	BrushTypeLibrary = "Brush Types",
	ProjectReferenceLibrary = "Project References",
	PropertyTypeLibrary = "Property Types",
}

local Project = {}

function Project:register()
	core2.projects[self.Name] = self
	filesystem.mount(self.Name, self.__path)
end

function Project:load_project_references()
	for _, ref in ipairs(self.ProjectReferenceLibrary or {}) do
		local refpath = filesystem.joinPaths(self.__path, ref)
		assert(pcall(Project.load, refpath), string.format("Failed to load project reference: %s", refpath))
	end
end

function Project:load_property_types()
	for name, prop in pairs(self.PropertyTypeLibrary or {}) do
		core.registerPropertyType(setmetatable({ Name=name }, { __index=prop }))
	end
end

function Project.load(path)
	local project = setmetatable({ __path = path }, { __index = Project })
	local projectfile = filesystem.joinPaths(path, "package.lua")
	assert(filesystem.fileExists(projectfile), string.format("Project file not found: %s", projectfile))
	local chunk, err = loadfile(projectfile, "t", project)
	assert(chunk, string.format("Failed to load project file: %s", err))
	assert(pcall(chunk), "Error while executing project file: "..projectfile)
	assert(project.Name, "Project file must define a Name field")

	if core2.projects[project.Name] then return nil end

	project:register()
	project:load_project_references()
	project:load_property_types()

	return project
end

function core2.load_project(path)
	return Project.load(path)
end