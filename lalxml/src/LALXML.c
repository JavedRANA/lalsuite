/*
 *  Copyright (C) 2007 Jolien Creighton
 *  Copyright (C) 2009 Oliver Bock
 *  Copyright (C) 2009 Reinhard Prix
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with with program; see the file COPYING. If not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 */

/**
 * \ingroup LALXML_h
 */

#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlschemas.h>

#include <lal/LALStdio.h>
#include <lal/XLALError.h>
#include <lal/LALMalloc.h>
#include <lal/LALXML.h>


/* ---------- private prototypes ---------- */
INT4 XLALValidateDocument(const xmlDocPtr xmlDocument, const xmlSchemaValidCtxtPtr xmlSchemaValidator);

/* ---------- function definitions ---------- */

static void print_element_names(xmlNode *node)
{
    xmlNode *cur;
    for (cur = node; cur; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE)
            printf("node type: Element, name: %s\n", cur->name);
        print_element_names(cur->children);
    }
    return;
}

int XLALXMLFilePrintElements(const char *fname)
{
    xmlDoc  *doc;

    /* make sure that the shared library is the same as the
     * library version the code was compiled against */
    LIBXML_TEST_VERSION

    if (!(doc = xmlReadFile(fname, NULL, 0)))
        XLAL_ERROR(XLAL_EIO);
    print_element_names(xmlDocGetRootElement(doc));
    xmlFreeDoc(doc);
    xmlCleanupParser(); /* free global variables in parser */
    return 0;
}


/**
 * Find next node element of given name within the siblings of the given xmlNode.
 *
 * Note: if the startNode already complies with the constraints, it is returned, ie
 * this function does not do stepping from one valid node to the next!
 *
 * Note: don't free the returned nodePointer!
 */
const xmlNode *
XLALfindNextNamedNode ( const xmlNode *startNode, const xmlChar *nodeName )
{
  const xmlNode *tmp;

  /* check input consistency */
  if ( !startNode || !nodeName ) {
    XLALPrintError ("%s: invalid NULL input.\n", __func__ );
    XLAL_ERROR_NULL ( XLAL_EINVAL );
  }

  tmp = startNode;
  while ( tmp && (tmp->type != XML_ELEMENT_NODE) && (xmlStrcmp(tmp->name, nodeName)) )
    tmp = tmp->next;

  return tmp;	/* either NULL or the desired named NODE */

} /* XLALfindNextNamedNode() */

/**
 * Count number of named nodes siblings of given startNode.
 *
 * Note: if the startNode already complies with the constraints, it is counted as well,
 * but no previous siblings are counted, ie the count starts at startNode.
 */
int
XLALcountNamedNodes ( const xmlNode *startNode, const xmlChar *nodeName, UINT4 *count )
{
  UINT4 counter;
  const xmlNode *tmp;

  /* check input consistency */
  if ( !startNode || !nodeName || !count ) {
    XLALPrintError ("%s: invalid NULL input.\n", __func__ );
    XLAL_ERROR ( XLAL_EINVAL );
  }

  counter = 0;
  tmp = startNode;

  while ( tmp && ( tmp = XLALfindNextNamedNode ( tmp, nodeName )) != NULL )
    {
      counter ++;
      tmp = tmp->next;	/* step forward to next element */
    }

  (*count) = counter;

  return XLAL_SUCCESS;

} /* XLALcountNamedNodes() */


/**
 * \brief Performs a XPath search on a XML document to retrieve the content of a single %node
 *
 * This function searches the given XML document using the given XPath statement.
 * The XPath statement \b must be specified in such a way that at most a single %node
 * will be found.
 *
 * \param xmlDocument [in] The XML document to be searched
 * \param xpath [in] The XPath statement to be used to search the given XML document
 * \param xmlNsVector [in] Vector of namespaces to be registered for XPath search
 *
 * \return A pointer to a \c xmlChar that holds the content (string) of the %node
 * specified by the given XPath statement. The content will be encoded in UTF-8.
 * In case of an error, a null-pointer is returned.\n
 * \b Important: the caller is responsible to free the allocated memory (when the
 * string isn't needed anymore) using \c xmlFree.
 *
 * \sa XLALGetSingleVOTResourceParamValue
 *
 * \author Oliver Bock\n
 * Albert-Einstein-Institute Hannover, Germany
 */
xmlChar * XLALGetSingleNodeContentByXPath(const xmlDocPtr xmlDocument, const char *xpath, const XML_NAMESPACE_VECTOR *xmlNsVector)
{
    /* set up local variables */
    xmlXPathContextPtr xpathCtx = NULL;
    xmlChar const *xmlNsPrefix = NULL;
    xmlChar const *xmlNsUrl = NULL;
    xmlChar *xpathExpr = NULL;
    xmlXPathObjectPtr xpathObj = NULL;
    xmlNodeSetPtr xmlNodes = NULL;
    xmlChar *nodeContent = NULL;
    INT4 nodeCount;
    int i;

    /* sanity checks */
    if(!xmlDocument) {
        XLALPrintError("Invalid input parameter: xmlDocument\n");
        XLAL_ERROR_NULL(XLAL_EINVAL);
    }
    if(!xpath || strlen(xpath) <= 0) {
        XLALPrintError("Invalid input parameter: xpath\n");
        XLAL_ERROR_NULL(XLAL_EINVAL);
    }

    /* prepare xpath context */
    xpathCtx = xmlXPathNewContext(xmlDocument);
    if(xpathCtx == NULL) {
        XLALPrintError("XPath context instantiation failed\n");
        XLAL_ERROR_NULL(XLAL_EFAILED);
    }

    /* register namespaces */
    if(xmlNsVector) {
        for(i = 0; i < xmlNsVector->count; ++i) {
            xmlNsPrefix = xmlNsVector->items[i].prefix;
            xmlNsUrl= xmlNsVector->items[i].url;
            if(xmlXPathRegisterNs(xpathCtx, xmlNsPrefix, xmlNsUrl)) {
                /* clean up */
                xmlXPathFreeContext(xpathCtx);
                XLALPrintError("XPath namespace registration failed: %s=%s\n", xmlNsPrefix, xmlNsUrl);
                XLAL_ERROR_NULL(XLAL_EFAILED);
            }
        }
    }

    /* prepare xpath expression */
    xpathExpr = xmlCharStrdup(xpath);
    if(xpathExpr == NULL) {
        /* clean up */
        xmlXPathFreeContext(xpathCtx);
        XLALPrintError("XPath statement preparation failed\n");
        XLAL_ERROR_NULL(XLAL_EFAILED);
    }

    /* run xpath query */
    xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
    if(xpathObj == NULL) {
        /* clean up */
        xmlFree(xpathExpr);
        xmlXPathFreeContext(xpathCtx);
        XLALPrintError("XPath evaluation failed\n");
        XLAL_ERROR_NULL(XLAL_EFAILED);
    }

    /* retrieve node set returned by xpath query */
    xmlNodes = xpathObj->nodesetval;

    /* how many nodes did we find? */
    nodeCount = (xmlNodes) ? xmlNodes->nodeNr : 0;
    if(nodeCount <= 0) {
        /* clean up */
        xmlXPathFreeObject(xpathObj);
        xmlFree(xpathExpr);
        xmlXPathFreeContext(xpathCtx);
        XLALPrintError("XPath search didn't return any nodes\n");
        XLAL_ERROR_NULL(XLAL_EDOM);
    }
    else if(nodeCount > 1) {
        /* clean up */
        xmlXPathFreeObject(xpathObj);
        xmlFree(xpathExpr);
        xmlXPathFreeContext(xpathCtx);
        XLALPrintError("XPath search did return %i nodes where only 1 was expected\n", nodeCount);
        XLAL_ERROR_NULL(XLAL_EDOM);
    }
    else {
        nodeContent = xmlNodeListGetString(xmlDocument, xmlNodes->nodeTab[0]->xmlChildrenNode, 1);
    }

    /* clean up */
    xmlXPathFreeObject(xpathObj);
    xmlFree(xpathExpr);
    xmlXPathFreeContext(xpathCtx);

    /* return node content (needs to be xmlFree'd by caller!!!) */
    return nodeContent;
}


/**
 * \brief Validates the given XML document using its internal schema definition
 *
 * This uses the internal schema definition of the given document to determine its
 * validity. The schema definition has to be specified in the attribute xsi:schemaLocation
 * (or xsi:noNamespaceSchemaLocation) of the document's root element.
 *
 * \param xmlDocument [in] The XML document to be validated
 *
 * \return \c XLAL_SUCCESS if the document is valid and \c XLAL_FAILURE if it's invalid.
 * In case of an error a \c XLAL error code is returned.
 *
 * \sa XLALValidateDocumentByExternalSchema
 * \sa XLALValidateDocument
 *
 * \author Oliver Bock\n
 * Albert-Einstein-Institute Hannover, Germany
 */
INT4 XLALValidateDocumentByInternalSchema(const xmlDocPtr xmlDocument)
{
    /* set up local variables */
    xmlNodePtr xmlRootNode = NULL;
    xmlChar *xmlSchemaLocation = NULL;
    INT4 result;

    /* sanity checks */
    if(!xmlDocument) {
        XLALPrintError("Invalid input parameter: xmlDocument\n");
        XLAL_ERROR(XLAL_EINVAL);
    }

    /* get schema instance location */
    xmlRootNode = xmlDocGetRootElement(xmlDocument);
    if(!xmlRootNode) {
        XLALPrintError("Root element retrieval failed!\n");
        XLAL_ERROR(XLAL_EFAILED);
    }
    xmlSchemaLocation = xmlGetNsProp(xmlRootNode,
                                     CAST_CONST_XMLCHAR("noNamespaceSchemaLocation"),
                                     CAST_CONST_XMLCHAR("http://www.w3.org/2001/XMLSchema-instance"));
    if(!xmlSchemaLocation) {
        XLALPrintError("Schema location retrieval failed!\n");
        XLAL_ERROR(XLAL_EFAILED);
    }

    /* prepare and run validator */
    result = XLALValidateDocumentByExternalSchema(xmlDocument, xmlSchemaLocation);

    /* clean up */
    xmlFree(xmlSchemaLocation);

    return result;
}


/**
 * \brief Validates the given XML document using the given external schema definition
 *
 * This function uses the specified URL to retrieve the schema definition file (XSD)
 * which in turn will be used to validate the given document.
 *
 * \param xmlDocument [in] The XML document to be validated
 * \param schemaUrl [in] The URL to be used for schema definition file (XSD) retrieval
 *
 * \return \c XLAL_SUCCESS if the document is valid and \c XLAL_FAILURE if it's invalid.
 * In case of an error a \c XLAL error code is returned.
 *
 * \sa XLALValidateDocumentByInternalSchema
 * \sa XLALValidateDocument
 *
 * \author Oliver Bock\n
 * Albert-Einstein-Institute Hannover, Germany
 */
INT4 XLALValidateDocumentByExternalSchema(const xmlDocPtr xmlDocument, const xmlChar *schemaUrl)
{
    /* set up local variables */
    xmlSchemaParserCtxtPtr xmlSchemaParser = NULL;
    xmlSchemaPtr xmlSchemaInstance = NULL;
    xmlSchemaValidCtxtPtr xmlSchemaValidator = NULL;
    INT4 result;

    /* sanity checks */
    if(!xmlDocument) {
        XLALPrintError("Invalid input parameter: xmlDocument\n");
        XLAL_ERROR(XLAL_EINVAL);
    }
    if(!schemaUrl) {
        XLALPrintError("Invalid input parameter: schemaUrl\n");
        XLAL_ERROR(XLAL_EINVAL);
    }

    /* retrieve schema and prepare parser */
    xmlSchemaParser = xmlSchemaNewParserCtxt((const char*)schemaUrl);
    if(!xmlSchemaParser) {
            XLALPrintError("Schema parser creation failed!\n");
            XLAL_ERROR(XLAL_EFAILED);
    }

    /* parse schema and prepare validator */
    xmlSchemaInstance = xmlSchemaParse(xmlSchemaParser);
    if(!xmlSchemaInstance) {
        /* clean up */
        xmlSchemaFreeParserCtxt(xmlSchemaParser);
        XLALPrintError("Schema parsing failed!\n");
        XLAL_ERROR(XLAL_EFAILED);
    }
    xmlSchemaValidator = xmlSchemaNewValidCtxt(xmlSchemaInstance);
    if(!xmlSchemaValidator) {
        /* clean up */
        xmlSchemaFreeParserCtxt(xmlSchemaParser);
        xmlSchemaFree(xmlSchemaInstance);
        XLALPrintError("Schema validator creation failed!\n");
        XLAL_ERROR(XLAL_EFAILED);
    }

    /* prepare and run validator */
    result = XLALValidateDocument(xmlDocument, xmlSchemaValidator);

    /* clean up */
    xmlSchemaFreeValidCtxt(xmlSchemaValidator);
    xmlSchemaFree(xmlSchemaInstance);
    xmlSchemaFreeParserCtxt(xmlSchemaParser);

    return result;
}


/**
 * \brief Validates the given XML document using the given schema validation context
 *
 * This function should not be used directly. Please refer to XLALValidateDocumentByInternalSchema
 *
 * \param xmlDocument [in] The XML document to be validated
 * \param xmlSchemaValidator [in] The schema validation context to be used
 *
 * \return \c XLAL_SUCCESS if the document is valid and \c XLAL_FAILURE if it's invalid.
 * In case of an error a \c XLAL error code is returned.
 *
 * \sa XLALValidateDocumentByInternalSchema
 * \sa XLALValidateDocumentByExternalSchema
 *
 * \author Oliver Bock\n
 * Albert-Einstein-Institute Hannover, Germany
 */
INT4 XLALValidateDocument(const xmlDocPtr xmlDocument, const xmlSchemaValidCtxtPtr xmlSchemaValidator)
{
    /* set up local variables */
    int result;

    /* sanity checks */
    if(!xmlDocument) {
        XLALPrintError("Invalid input parameter: xmlDocument\n");
        XLAL_ERROR(XLAL_EINVAL);
    }
    if(!xmlSchemaValidator) {
        XLALPrintError("Invalid input parameter: xmlSchemaValidator\n");
        XLAL_ERROR(XLAL_EINVAL);
    }

    /* validate document */
    result = xmlSchemaValidateDoc(xmlSchemaValidator, xmlDocument);
    if(result == 0) {
        return XLAL_SUCCESS;
    }
    else if(result == -1) {
        XLALPrintError("Document validation failed due to an internal error!\n");
        XLAL_ERROR(XLAL_EFAILED);
    }
    else {
        return XLAL_FAILURE;
    }
}


/**
 * \brief Assigns the given namespace to the given root element and all its children
 *
 * This function iterates recursively over the given root element and all its child
 * elements. Every element is assigned to the given namespace.\n
 * \n
 * \b Note: this functionality is seemingly not provided by \c libxml2. The closest candidates
 * are \c xmlReconciliateNs() and \c xmlDOMWrapReconcileNamespaces() but they don't work in
 * this scenario (bottom-up document construction)!
 *
 * \param xmlRootElement [in]
 * \param xmlNamespace [in]
 *
 * \return \c XLAL_SUCCESS if the namespace could be successfully assigned to all elements.
 * Please note that this function can't return anything else than XLAL_SUCCESS.
 *
 * \sa XLALCreateVOTDocumentFromTree
 *
 * \author Oliver Bock\n
 * Albert-Einstein-Institute Hannover, Germany
 */
INT4 XLALReconcileDefaultNamespace(const xmlNodePtr xmlRootElement, const xmlNsPtr xmlNamespace)
{
    /* set up local variables */
    xmlNodePtr xmlCurrentNode = NULL;

    /* iterate over the root element and all its children */
    for(xmlCurrentNode = xmlRootElement; xmlCurrentNode != NULL; xmlCurrentNode = xmlCurrentNode->next) {
        if (xmlCurrentNode->type == XML_ELEMENT_NODE) {
            /* can't be checked for errors */
            xmlSetNs(xmlCurrentNode, xmlNamespace);
        }

        /* recurse into next child (tree) */
        XLALReconcileDefaultNamespace(xmlCurrentNode->children, xmlNamespace);
    }

    return XLAL_SUCCESS;
}


/**
 * Convert a general (but complete!) XML-string into an xmlDoc representation.
 *
 * \author Reinhard Prix\n
 * Albert-Einstein-Institute Hannover, Germany
 */
xmlDocPtr
XLALXMLString2Doc ( const char *xmlString )
{
  xmlDocPtr ret = NULL;

  /* check input consistency */
  if ( !xmlString ) {
    XLALPrintError ("%s: invalid NULL input.\n", __func__ );
    XLAL_ERROR_NULL ( XLAL_EINVAL );
  }

  /* parse XML document */
  if ( (ret = xmlReadMemory((const char*)xmlString, strlen((const char*)xmlString), NULL, "UTF-8", 0)) == NULL ) {
    XLALPrintError ( "%s: XML document parsing xmlReadMemory() failed!\n", __func__ );
    XLAL_ERROR_NULL ( XLAL_EINVAL );
  }

  /* clean up */
  xmlCleanupParser();

  return ret;

} /* XLALXMLString2Doc() */


/**
 * Convert the given xmlDoc representation of a VOTable into a string representation,
 * eg for storing the XML into a file.
 *
 * Note: the returned string must be free'ed using \c xmlFree !
 *
 * \author Reinhard Prix\n
 * Albert-Einstein-Institute Hannover, Germany
 */
xmlChar *
XLALXMLDoc2String ( xmlDoc *xmlDocument )
{
  /* set up local variables */
  int xmlStringBufferSize = 0;
  xmlChar *xmlString = NULL;

  /* check input consistency */
  if ( !xmlDocument ) {
    XLALPrintError ("%s: invalid NULL input 'xmlDocument'\n", __func__ );
    XLAL_ERROR_NULL ( XLAL_EINVAL );
  }

  /* prepare XML serialization (here: indentation) */
  xmlThrDefIndentTreeOutput(1);

  /* dump document to a string buffer */
  xmlDocDumpFormatMemory ( (xmlDocPtr)xmlDocument, &xmlString, &xmlStringBufferSize, 1);
  if(xmlStringBufferSize <= 0) {
    XLALPrintError ("%s: XML document dump xmlDocDumpFormatMemory() failed!\n", __func__);
    XLAL_ERROR_NULL ( XLAL_EFAILED );
  }

  return xmlString;

} /* XLALXMLDoc2String() */

