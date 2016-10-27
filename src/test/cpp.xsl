<?xml version="1.0"?>
<!-- 
 *************************************************************************
 ** Copyright (C) 2013 Jan Pedersen <jp@jp-embedded.com>
 ** 
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 ** 
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 ** 
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************
-->
<xsl:stylesheet
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:conf="http://www.w3.org/2005/scxml-conformance"
    version="1.0">

<!-- Copy everything that doesn't match other rules -->
<xsl:template match="/ | @* | node()">
  <xsl:copy>
    <xsl:apply-templates select="@* | node()"/>
  </xsl:copy>
</xsl:template>

<!-- Success criteria -->
<xsl:template match="//@conf:targetpass">
	<xsl:attribute name="target">pass</xsl:attribute>
</xsl:template>

<xsl:template match="conf:pass">
  <state xmlns="http://www.w3.org/2005/07/scxml" id="pass">
  </state>
</xsl:template>

<!-- Failure criteria -->
<xsl:template match="//@conf:targetfail">
	<xsl:attribute name="target">fail</xsl:attribute>
</xsl:template>

<xsl:template match="conf:fail">
  <state xmlns="http://www.w3.org/2005/07/scxml" id="fail">
  </state>
</xsl:template>

<!-- datamodel -->
<xsl:template match="//@conf:datamodel">
	<xsl:attribute name="datamodel">cplusplus</xsl:attribute>
</xsl:template>

<!-- creates id for <data> element. -->
<xsl:template match="//@conf:id">
	<xsl:attribute name="id">Var<xsl:value-of select="." /></xsl:attribute>
</xsl:template>

<!-- creates expr for <expr> element. -->
<xsl:template match="//@conf:expr">
	<xsl:attribute name="expr"><xsl:value-of select="." /></xsl:attribute>
</xsl:template>

<!-- creates expr for <location> element. -->
<xsl:template match="//@conf:location">
	<xsl:attribute name="location">Var<xsl:value-of select="." /></xsl:attribute>
</xsl:template>

<!-- creates name for <param>. -->
<xsl:template match="//@conf:name">
	<xsl:attribute name="name">Var<xsl:value-of select="." /></xsl:attribute>
</xsl:template>

<!-- incrementID. -->
<xsl:template match="conf:incrementID">
	<script>++m.Var<xsl:value-of select="@id"/>;</script>
</xsl:template>

<!-- transition condition - value -->
<xsl:template match="//@conf:idVal">
	<xsl:attribute name="cond">Var<xsl:value-of select="concat(substring-before(.,'='),'==',substring-after(.,'='))" /></xsl:attribute>
</xsl:template>

</xsl:stylesheet>
