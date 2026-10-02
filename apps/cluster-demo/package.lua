Name = "ClusterDemo"
StartupScreen = "ClusterDemo/Screens/Cluster"
WindowWidth = 1920
WindowHeight = 720
ScreenLibrary = { IsExternal = true }
SystemMessages = {
	{ Message="KeyDown", Key="q", Command="return" },
	{ Message="WindowClosed", Command="return" },
	{ Message="RequestReload", Command="window:refresh()" },
}
EnginePlugins = {
	{ Name="orca.UIKit" },
	{ Name="orca.SceneKit" },
}
-- Shader uniforms. tools/blender-export.py reports any that are missing here.
PropertyTypes = {
	{ Name = "Color0", Category = "Uniform", DataType = "Color" },
	{ Name = "Color1", Category = "Uniform", DataType = "Color" },
	{ Name = "Color2", Category = "Uniform", DataType = "Color" },
	{ Name = "ColorDark", Category = "Uniform", DataType = "Color" },
	{ Name = "ColorLight", Category = "Uniform", DataType = "Color" },
	{ Name = "ColorOff", Category = "Uniform", DataType = "Color" },
	{ Name = "ColorOn", Category = "Uniform", DataType = "Color" },
	{ Name = "EdgeDark", Category = "Uniform", DataType = "Float" },
	{ Name = "FadeIn", Category = "Uniform", DataType = "Float" },
	{ Name = "FadeOut", Category = "Uniform", DataType = "Float" },
	{ Name = "InnerFade", Category = "Uniform", DataType = "Float" },
	{ Name = "Intensity", Category = "Uniform", DataType = "Float" },
	{ Name = "Level", Category = "Uniform", DataType = "Float" },
	{ Name = "Lobes", Category = "Uniform", DataType = "Float" },
	{ Name = "Phase", Category = "Uniform", DataType = "Float" },
	{ Name = "Pos0", Category = "Uniform", DataType = "Float" },
	{ Name = "Pos1", Category = "Uniform", DataType = "Float" },
	{ Name = "Pos2", Category = "Uniform", DataType = "Float" },
	{ Name = "Power", Category = "Uniform", DataType = "Float" },
	{ Name = "RingContrast", Category = "Uniform", DataType = "Float" },
	{ Name = "Rings", Category = "Uniform", DataType = "Float" },
	{ Name = "Sharpness", Category = "Uniform", DataType = "Float" },
	{ Name = "Soft", Category = "Uniform", DataType = "Float" },
	{ Name = "Tail", Category = "Uniform", DataType = "Float" },
	{ Name = "Tint", Category = "Uniform", DataType = "Color" },
}
