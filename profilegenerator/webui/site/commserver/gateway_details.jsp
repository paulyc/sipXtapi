<%--
 -
 - Copyright (c) 2003 Pingtel Corp.  (work in progress)
 -
 - This is an unpublished work containing Pingtel Corporation's confidential
 - and proprietary information.  Disclosure, use or reproduction without
 - written authorization of Pingtel Corp. is prohibited.
 -
 - 
 - 
 - Author: 
 --%>
<%@ page errorPage="error/error.jsp" %>
<%@ page import="com.pingtel.commserver.beans.*" %>
<%@ page import="java.io.*" %>
<%@ page language="Java" %>
<%
    // This is not inline with MVC design and shouldn't be considered an
    // example of how to write new pages.  My excuse is that infratructure
    // to support MVC is too difficult to maintain.

    String userMessage = null;
    String errorMessage = null;
    XmlEditor xml = new XmlEditor();

    if ("save".equalsIgnoreCase(request.getParameter("action")))
    {
	xml.setContent(request.getParameter("content"));
	if (xml.isValid())
        {
	    try
            {
	        xml.save(ProcessDestinationsBean.getAuthRulesFilename());
	        userMessage = "Save successful";
            } 
	    catch (IOException ioe)
            {
	        errorMessage = "Internal error saving file: " + ioe.getMessage();
            }
	} 
	else
        {
	    errorMessage = xml.getErrorMessage();
        }
    }
    // default and reload
    else
    {
        xml.load(ProcessDestinationsBean.getAuthRulesFilename());
    }
%>
<html>
<head>
<link rel="stylesheet" href="../style/dms.css" type="text/css">
<script src="script/jsFunctions.js"></script>
</head>
<body class="bglight">

<%
  if (errorMessage != null) 
  {
%><p id="error"><%= errorMessage %>
<%
  }
%>
<%
  if (userMessage != null) 
  {
%><p class="msgtext"><%= userMessage %></p>
<%
  }
%>
<table width="600">
    <tr>
        <td width="50%" align="left">
            <h1 class="list">Dial Plan Authorization Rules</h1>
        </td>
        <td width="50%" align="right">
            <a href="#" onclick="window.parent.MM_openBrWindow('/pds/commserver/help/commserver/WebHelp/configsrvr.htm#gateways.htm','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')" class="formtext">Help</a>
        </td>
    </tr>
    <tr>
        <td colspan="2"><hr class="dms"></td>
    </tr>
</table>
<form name="inputform" action="gateway_details.jsp" method="post">
<input type="submit" name="action" value="Save">
<input type="submit" name="action" value="Reload"><br/>
<textarea name="content" rows="35" cols="90"><%= xml.getContent() %></textarea>
</form>
</body>
</html>