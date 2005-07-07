<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!--
  - Select and copy context of select node
  -->
<xsl:template match="vxml">
  <xsl:apply-templates select="form[@id='aa']/field[@name='menu']/filled/if" mode="content"/>	
</xsl:template>

<xsl:template match='text()|*' mode="content">
  <xsl:copy>
    <xsl:copy-of select="@*"/>
    <xsl:apply-templates  mode="content"/>
  </xsl:copy>
</xsl:template>

</xsl:stylesheet>
