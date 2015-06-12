<%@ page session="false" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %> 

[HTTP Headers]

<c:forEach items="${header}" var="requestHeader">
* "${requestHeader.key}" => "${requestHeader.value}"
</c:forEach>

[POST Parameters]

<c:forEach items="${param}" var="postParam">
* "${postParam.key}" => "${postParam.value}"
</c:forEach>
