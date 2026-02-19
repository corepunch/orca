"""
Plugin architecture for conv-module.py.

Each plugin is a subclass of Plugin defined in its own file.
When a subclass is defined (i.e. when its module is imported),
it is automatically added to the registry via __init_subclass__.

Usage:
    from plugins import Plugin, get_plugins

    # Load all built-in plugins:
    import plugins.dtd_writer
    import plugins.header_writer
    import plugins.html_writer
    import plugins.properties_writer
    import plugins.export_writer

    # Iterate over registered plugin instances:
    for plugin in get_plugins():
        plugin.some_hook(...)
"""

_registry: list = []


class Plugin:
    """Base class for all writer plugins.

    Every concrete subclass is automatically instantiated and appended
    to the global registry the moment its class body is executed, so the
    only thing a plugin file needs to do is::

        from plugins import Plugin

        class MyPlugin(Plugin):
            def on_something(self, root, node):
                ...

    No manual registration call is required.
    """

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        _registry.append(cls())


def get_plugins() -> list:
    """Return a snapshot of all currently registered plugin instances."""
    return list(_registry)
