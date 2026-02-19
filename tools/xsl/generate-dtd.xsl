<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="text" encoding="UTF-8"/>
    
    <!-- Main template -->
    <xsl:template match="/">
        <xsl:apply-templates select="//component"/>
        <xsl:apply-templates select="//resource"/>
        <xsl:apply-templates select="//property"/>
    </xsl:template>
    
    <!-- Component DTD elements -->
    <xsl:template match="component">
        <xsl:text>&lt;!ELEMENT </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text> (%NodeElements;)*&gt;&#10;</xsl:text>
        
        <xsl:text>&lt;!ATTLIST </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text> %NodeAttribs;&#10;</xsl:text>
        
        <!-- Add properties as attributes -->
        <xsl:for-each select="property">
            <xsl:text>    </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text> CDATA #IMPLIED</xsl:text>
            <xsl:if test="position() != last()">
                <xsl:text>&#10;</xsl:text>
            </xsl:if>
        </xsl:for-each>
        <xsl:text>&gt;&#10;&#10;</xsl:text>
    </xsl:template>
    
    <!-- Resource DTD elements -->
    <xsl:template match="resource">
        <xsl:text>&lt;!-- Resource: </xsl:text>
        <xsl:value-of select="@type"/>
        <xsl:text> --&gt;&#10;</xsl:text>
    </xsl:template>
    
    <!-- Property handling (already handled in component) -->
    <xsl:template match="property"/>
    
</xsl:stylesheet>
