"""
HTML Plugin - Generates HTML documentation
"""

from . import BasePlugin

class HtmlPlugin(BasePlugin):
	"""Plugin for generating HTML documentation"""
	def __init__(self):
		# HTML generation uses global state (g_html, g_sidebar, g_content)
		# This plugin doesn't need a file handle as it's managed separately
		pass
	
	def write_to_global(self, element_type, *args, **kwargs):
		"""Write to the global HTML structure"""
		# This would be used for adding elements to g_sidebar, g_content, etc.
		pass
