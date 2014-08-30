#include "data/Storage.h"

#include "data/graph/filter/GraphFilterConductor.h"
#include "data/graph/token_component/TokenComponentConst.h"
#include "data/graph/token_component/TokenComponentDataType.h"
#include "data/graph/token_component/TokenComponentStatic.h"
#include "data/graph/SubGraph.h"
#include "data/location/TokenLocation.h"
#include "data/location/TokenLocationFile.h"
#include "data/location/TokenLocationLine.h"
#include "data/parser/ParseFunction.h"
#include "data/parser/ParseLocation.h"
#include "data/parser/ParseTypeUsage.h"
#include "data/parser/ParseVariable.h"
#include "data/query/QueryTree.h"
#include "data/type/DataType.h"
#include "utility/logging/logging.h"
#include "utility/utilityString.h"

Storage::Storage()
{
}

Storage::~Storage()
{
}

void Storage::clear()
{
	m_graph.clear();
	m_locationCollection.clear();
}

void Storage::logGraph() const
{
	LOG_INFO_STREAM(<< '\n' << m_graph);
}

void Storage::logLocations() const
{
	LOG_INFO_STREAM(<< '\n' << m_locationCollection);
}


Id Storage::onTypedefParsed(
	const ParseLocation& location, const std::string& fullName, const ParseTypeUsage& underlyingType, AccessType access
){
	log("typedef", fullName + " -> " + underlyingType.dataType.getFullTypeName(), location);

	Node* node = m_graph.createNodeHierarchy(Node::NODE_TYPEDEF, fullName);
	addAccess(node, access);
	addTokenLocation(node, location);
	addTypeEdge(node, Edge::EDGE_TYPEDEF_OF, underlyingType);

	return node->getId();
}

Id Storage::onClassParsed(
	const ParseLocation& location, const std::string& fullName, AccessType access, const ParseLocation& scopeLocation
){
	log("class", fullName, location);

	Node* node = m_graph.createNodeHierarchy(Node::NODE_CLASS, fullName);
	addAccess(node, access);
	addTokenLocation(node, location);
	addTokenLocation(node, scopeLocation, true);

	return node->getId();
}

Id Storage::onStructParsed(
	const ParseLocation& location, const std::string& fullName, AccessType access, const ParseLocation& scopeLocation
){
	log("struct", fullName, location);

	Node* node = m_graph.createNodeHierarchy(Node::NODE_STRUCT, fullName);
	addAccess(node, access);
	addTokenLocation(node, location);
	addTokenLocation(node, scopeLocation, true);

	return node->getId();
}

Id Storage::onGlobalVariableParsed(const ParseLocation& location, const ParseVariable& variable)
{
	log("global", variable.fullName, location);

	Node* node = m_graph.createNodeHierarchy(Node::NODE_GLOBAL_VARIABLE, variable.fullName);

	if (variable.isStatic)
	{
		node->addComponentStatic(std::make_shared<TokenComponentStatic>());
	}

	addTypeEdge(node, Edge::EDGE_TYPE_OF, variable.type);
	addTokenLocation(node, location);

	return node->getId();
}

Id Storage::onFieldParsed(const ParseLocation& location, const ParseVariable& variable, AccessType access)
{
	log("field", variable.fullName, location);

	Node* node = m_graph.createNodeHierarchy(Node::NODE_FIELD, variable.fullName);

	if (!node->getMemberEdge())
	{
		LOG_ERROR("Field is not a member of anything.");
	}

	if (variable.isStatic)
	{
		node->addComponentStatic(std::make_shared<TokenComponentStatic>());
	}

	if (access == ACCESS_NONE)
	{
		LOG_ERROR("Field needs to have access type [public, protected, private] but has none.");
	}
	addAccess(node, access);

	addTypeEdge(node, Edge::EDGE_TYPE_OF, variable.type);
	addTokenLocation(node, location);

	return node->getId();
}

Id Storage::onFunctionParsed(
	const ParseLocation& location, const ParseFunction& function, const ParseLocation& scopeLocation
){
	log("function", function.fullName, location);

	Node* node = m_graph.createNodeHierarchyWithDistinctSignature(
		Node::NODE_FUNCTION, function.fullName, ParserClient::functionSignatureStr(function)
	);

	addTokenLocation(node, location);
	addTokenLocation(node, scopeLocation, true);

	addTypeEdge(node, Edge::EDGE_RETURN_TYPE_OF, function.returnType);
	for (const ParseTypeUsage& parameter : function.parameters)
	{
		addTypeEdge(node, Edge::EDGE_PARAMETER_TYPE_OF, parameter);
	}

	return node->getId();
}

Id Storage::onMethodParsed(
	const ParseLocation& location, const ParseFunction& method, AccessType access, AbstractionType abstraction,
	const ParseLocation& scopeLocation
){
	log("method", method.fullName, location);

	Node* node = m_graph.createNodeHierarchyWithDistinctSignature(
		Node::NODE_METHOD, method.fullName, ParserClient::functionSignatureStr(method)
	);

	if (!node->getMemberEdge())
	{
		LOG_ERROR("Method is not a member of anything.");
	}

	if (method.isConst)
	{
		node->addComponentConst(std::make_shared<TokenComponentConst>());
	}

	if (method.isStatic)
	{
		node->addComponentStatic(std::make_shared<TokenComponentStatic>());
	}

	if (access == ACCESS_NONE)
	{
		LOG_ERROR("Method needs to have access type [public, protected, private] but has none.");
	}
	addAccess(node, access);
	addAbstraction(node, abstraction);

	addTokenLocation(node, location);
	addTokenLocation(node, scopeLocation, true);

	addTypeEdge(node, Edge::EDGE_RETURN_TYPE_OF, method.returnType);
	for (const ParseTypeUsage& parameter : method.parameters)
	{
		addTypeEdge(node, Edge::EDGE_PARAMETER_TYPE_OF, parameter);
	}

	return node->getId();
}

Id Storage::onNamespaceParsed(
	const ParseLocation& location, const std::string& fullName, const ParseLocation& scopeLocation
){
	log("namespace", fullName, location);

	Node* node = m_graph.createNodeHierarchy(Node::NODE_NAMESPACE, fullName);
	addTokenLocation(node, location);
	addTokenLocation(node, scopeLocation, true);

	return node->getId();
}

Id Storage::onEnumParsed(
	const ParseLocation& location, const std::string& fullName, AccessType access, const ParseLocation& scopeLocation
){
	log("enum", fullName, location);

	Node* node = m_graph.createNodeHierarchy(Node::NODE_ENUM, fullName);
	addAccess(node, access);
	addTokenLocation(node, location);
	addTokenLocation(node, scopeLocation, true);

	return node->getId();
}

Id Storage::onEnumFieldParsed(const ParseLocation& location, const std::string& fullName)
{
	log("enum field", fullName, location);

	Node* node = m_graph.createNodeHierarchy(Node::NODE_FIELD, fullName);
	addTokenLocation(node, location);

	return node->getId();
}

Id Storage::onInheritanceParsed(
	const ParseLocation& location, const std::string& fullName, const std::string& baseName, AccessType access
){
	log("inheritance", fullName + " : " + baseName, location);

	Node* node = m_graph.createNodeHierarchy(fullName);
	Node* baseNode = m_graph.createNodeHierarchy(baseName);

	Edge* edge = m_graph.createEdge(Edge::EDGE_INHERITANCE, node, baseNode);
	edge->addComponentAccess(std::make_shared<TokenComponentAccess>(convertAccessType(access)));

	addTokenLocation(edge, location);

	return edge->getId();
}

Id Storage::onCallParsed(const ParseLocation& location, const ParseFunction& caller, const ParseFunction& callee)
{
	log("call", caller.fullName + " -> " + callee.fullName, location);

	Node* callerNode =
		m_graph.createNodeHierarchyWithDistinctSignature(caller.fullName, ParserClient::functionSignatureStr(caller));
	Node* calleeNode =
		m_graph.createNodeHierarchyWithDistinctSignature(callee.fullName, ParserClient::functionSignatureStr(callee));

	Edge* edge = m_graph.createEdge(Edge::EDGE_CALL, callerNode, calleeNode);

	addTokenLocation(edge, location);

	return edge->getId();
}

Id Storage::onCallParsed(const ParseLocation& location, const ParseVariable& caller, const ParseFunction& callee)
{
	log("call", caller.fullName + " -> " + callee.fullName, location);

	Node* callerNode =
		m_graph.createNodeHierarchy(caller.fullName);
	Node* calleeNode =
		m_graph.createNodeHierarchyWithDistinctSignature(callee.fullName, ParserClient::functionSignatureStr(callee));

	Edge* edge = m_graph.createEdge(Edge::EDGE_CALL, callerNode, calleeNode);

	addTokenLocation(edge, location);

	return edge->getId();
}

Id Storage::onFieldUsageParsed(const ParseLocation& location, const ParseFunction& user, const std::string& usedName)
{
	log("field usage", user.fullName + " -> " + usedName, location);

	Node* userNode =
		m_graph.createNodeHierarchyWithDistinctSignature(user.fullName, ParserClient::functionSignatureStr(user));
	Node* usedNode = m_graph.createNodeHierarchy(usedName);

	Edge* edge = m_graph.createEdge(Edge::EDGE_USAGE, userNode, usedNode);
	addTokenLocation(edge, location);

	return edge->getId();
}

Id Storage::onGlobalVariableUsageParsed(
		const ParseLocation& location, const ParseFunction& user, const std::string& usedName
){
	log("global usage", user.fullName + " -> " + usedName, location);

	Node* userNode =
		m_graph.createNodeHierarchyWithDistinctSignature(user.fullName, ParserClient::functionSignatureStr(user));;
	Node* usedNode = m_graph.createNodeHierarchy(usedName);

	Edge* edge = m_graph.createEdge(Edge::EDGE_USAGE, userNode, usedNode);
	addTokenLocation(edge, location);

	return edge->getId();
}

Id Storage::onTypeUsageParsed(const ParseTypeUsage& type, const ParseFunction& function)
{
	log("type usage", function.fullName + " -> " + type.dataType.getRawTypeName(), type.location);

	Node* functionNode =
		m_graph.createNodeHierarchyWithDistinctSignature(function.fullName, ParserClient::functionSignatureStr(function));
	Edge* edge = addTypeEdge(functionNode, Edge::EDGE_TYPE_USAGE, type);

	return edge->getId();
}

Id Storage::getIdForNodeWithName(const std::string& fullName) const
{
	Node* node = m_graph.getNode(fullName);
	return (node ? node->getId() : 0);
}

std::string Storage::getNameForNodeWithId(Id id) const
{
	Token* token = m_graph.getTokenById(id);

	if (!token)
	{
		return "";
	}

	if (token->isEdge())
	{
		return dynamic_cast<Edge*>(token)->getName();
	}
	else
	{
		return dynamic_cast<Node*>(token)->getFullName();
	}
}

std::vector<std::string> Storage::getNamesForNodesWithNamePrefix(const std::string& prefix) const
{
	std::vector<std::string> names;
	m_graph.forEachNode([&](Node* node){
		const std::string& nodeName = node->getFullName();
		if (utility::isPrefix(prefix, nodeName))
		{
			names.push_back(nodeName);
		}
	});

	return names;
}

std::vector<Id> Storage::getIdsOfNeighbours(const Id id) const
{
	std::vector<Id> result;

	Node* node = m_graph.findNode([&](Node* node){
		return node->getId() == id;
	});

	if (node != NULL)
	{
		std::map<Id, bool> addedIds;
		addedIds[id] = true;

		node->forEachEdge(
				[&result, &addedIds](Edge* e)
				{
					Id fromId = e->getFrom()->getId();
					if (addedIds.find(fromId) == addedIds.end())
					{
						result.push_back(fromId);
						addedIds[fromId] = true;
					}

					Id toId = e->getTo()->getId();
					if (addedIds.find(toId) == addedIds.end())
					{
						result.push_back(toId);
						addedIds[toId] = true;
					}
				}
			);
	}

	return result;
}

std::vector<std::tuple<Id, Id, Id>> Storage::getNeighbourEdgesOfNode(const Id id) const
{
	std::vector<std::tuple<Id, Id, Id>> result;

	Node* node = m_graph.findNode([&](Node* node){
		return node->getId() == id;
	});

	if (node != NULL)
	{
		node->forEachEdge(
				[&result](Edge* e)
				{
					result.push_back(std::tuple<Id, Id, Id>(e->getFrom()->getId(), e->getTo()->getId(), e->getId()));
				}
			);
	}

	return result;
}

std::vector<std::tuple<Id, Id, Id>> Storage::getMemberEdgesOfNode(const Id id) const
{
	return getEdgesOfTypeOfNode(id, Edge::EdgeType::EDGE_MEMBER);
}

std::vector<std::tuple<Id, Id, Id>> Storage::getUsageEdgesOfNode(const Id id) const
{
	return getEdgesOfTypeOfNode(id, Edge::EdgeType::EDGE_USAGE);
}

std::vector<std::tuple<Id, Id, Id>> Storage::getCallEdgesOfNode(const Id id) const
{
	return getEdgesOfTypeOfNode(id, Edge::EdgeType::EDGE_CALL);
}

std::vector<std::tuple<Id, Id, Id>> Storage::getTypeOfEdgesOfNode(const Id id) const
{
	return getEdgesOfTypeOfNode(id, Edge::EdgeType::EDGE_TYPE_OF);
}

std::vector<std::tuple<Id, Id, Id>> Storage::getReturnTypeOfEdgesOfNode(const Id id) const
{
	return getEdgesOfTypeOfNode(id, Edge::EdgeType::EDGE_RETURN_TYPE_OF);
}

std::vector<std::tuple<Id, Id, Id>> Storage::getParameterOfEdgesOfNode(const Id id) const
{
	return getEdgesOfTypeOfNode(id, Edge::EdgeType::EDGE_PARAMETER_TYPE_OF);
}

std::vector<std::tuple<Id, Id, Id>> Storage::getInheritanceEdgesOfNode(const Id id) const
{
	return getEdgesOfTypeOfNode(id, Edge::EdgeType::EDGE_INHERITANCE);
}

std::pair<Id, Id> Storage::getNodesOfEdge(const Id id) const
{
	std::pair<Id, Id> result;

	Edge* edge = m_graph.findEdge([&](Edge* edge){
		return edge->getId() == id;
	});

	if(edge != NULL)
	{
		result.first = edge->getFrom()->getId();
		result.second = edge->getTo()->getId();
	}

	return result;
}

bool Storage::checkTokenIsNode(const Id id) const
{
	Token* token = m_graph.findToken([&](Token* token){
		return token->getId() == id;
	});

	if(token != NULL)
	{
		return token->isNode();
	}

	return false;
}

std::vector<Id> Storage::getActiveTokenIdsForId(Id tokenId) const
{
	std::vector<Id> ret;
	Token* token = m_graph.getTokenById(tokenId);
	if (!token)
	{
		return ret;
	}

	Node* node;
	if (token->isEdge())
	{
		node = dynamic_cast<Edge*>(token)->getTo();
	}
	else
	{
		node = dynamic_cast<Node*>(token);
	}

	ret.push_back(node->getId());

	node->forEachEdge(
		[&node, &ret](Edge* e)
		{
			if (e->getTo() == node)
			{
				ret.push_back(e->getId());
			}
		}
	);

	return ret;
}

std::vector<Id> Storage::getLocationIdsForTokenIds(const std::vector<Id>& tokenIds) const
{
	std::vector<Id> ret;

	for (Id tokenId : tokenIds)
	{
		Token* token = m_graph.getTokenById(tokenId);
		if (!token)
		{
			continue;
		}

		ret.insert(ret.end(), token->getLocationIds().begin(), token->getLocationIds().end());
	}

	return ret;
}

std::vector<Id> Storage::getTokenIdsForQuery(std::string query) const
{
	QueryTree tree(query);
	GraphFilterConductor conductor;
	SubGraph outGraph;

	conductor.filter(&tree, &m_graph, &outGraph);

	LOG_INFO_STREAM(<< '\n' << tree << '\n' << outGraph);

	return outGraph.getTokenIds();
}

TokenLocationCollection Storage::getTokenLocationsForLocationIds(const std::vector<Id>& locationIds) const
{
	TokenLocationCollection ret;

	for (Id locationId: locationIds)
	{
		TokenLocation* location = m_locationCollection.findTokenLocationById(locationId);
		if (location->getOtherTokenLocation())
		{
			ret.addTokenLocationAsPlainCopy(location);
			ret.addTokenLocationAsPlainCopy(location->getOtherTokenLocation());
		}
	}

	return ret;
}

TokenLocationFile Storage::getTokenLocationsForFile(const std::string& filePath) const
{
	TokenLocationFile ret(filePath);

	TokenLocationFile* locationFile = m_locationCollection.findTokenLocationFileByPath(filePath);
	if (!locationFile)
	{
		return ret;
	}

	locationFile->forEachTokenLocation(
		[&](TokenLocation* tokenLocation) -> void
		{
			ret.addTokenLocationAsPlainCopy(tokenLocation);
		}
	);

	return ret;
}

TokenLocationFile Storage::getTokenLocationsForLinesInFile(
		const std::string& filePath, uint firstLineNumber, uint lastLineNumber
) const
{
	TokenLocationFile ret(filePath);

	TokenLocationFile* locationFile = m_locationCollection.findTokenLocationFileByPath(filePath);
	if (!locationFile)
	{
		return ret;
	}

	uint endLineNumber = locationFile->getTokenLocationLines().rbegin()->first;

	for (uint i = firstLineNumber; i <= endLineNumber; i++)
	{
		TokenLocationLine* locationLine = locationFile->findTokenLocationLineByNumber(i);
		if (!locationLine)
		{
			continue;
		}

		if (locationLine->getLineNumber() <= lastLineNumber)
		{
			locationLine->forEachTokenLocation(
				[&](TokenLocation* tokenLocation) -> void
				{
					ret.addTokenLocationAsPlainCopy(tokenLocation);
				}
			);
		}
		else
		{
			// Save start locations of TokenLocations that span accross the line range.
			locationLine->forEachTokenLocation(
				[&](TokenLocation* tokenLocation) -> void
				{
					if (tokenLocation->isEndTokenLocation() &&
						tokenLocation->getStartTokenLocation()->getLineNumber() < firstLineNumber)
					{
						ret.addTokenLocationAsPlainCopy(tokenLocation->getStartTokenLocation());
					}
				}
			);
		}
	}

	return ret;
}

Token* Storage::getTokenWithId(Id tokenId) const
{
	return m_graph.getTokenById(tokenId);
}

std::vector<TokenLocation*> Storage::getTokenLocationsForId(Id tokenId) const
{
	const std::vector<Id>& locationIds = getTokenWithId(tokenId)->getLocationIds();

	std::vector<TokenLocation*> result;
	for (Id locationId : locationIds)
	{
		result.push_back(m_locationCollection.findTokenLocationById(locationId));
	}

	return result;
}

TokenComponentAccess::AccessType Storage::convertAccessType(ParserClient::AccessType access) const
{
	switch (access)
	{
	case ACCESS_PUBLIC:
		return TokenComponentAccess::ACCESS_PUBLIC;
	case ACCESS_PROTECTED:
		return TokenComponentAccess::ACCESS_PROTECTED;
	case ACCESS_PRIVATE:
		return TokenComponentAccess::ACCESS_PRIVATE;
	case ACCESS_NONE:
		return TokenComponentAccess::ACCESS_NONE;
	}
}

TokenComponentAccess* Storage::addAccess(Node* node, ParserClient::AccessType access)
{
	if (access != ACCESS_NONE)
	{
		std::shared_ptr<TokenComponentAccess> ptr = std::make_shared<TokenComponentAccess>(convertAccessType(access));
		node->getMemberEdge()->addComponentAccess(ptr);
		return ptr.get();
	}
	return nullptr;
}

TokenComponentAbstraction::AbstractionType Storage::convertAbstractionType(ParserClient::AbstractionType abstraction) const
{
	switch (abstraction)
	{
	case ABSTRACTION_VIRTUAL:
		return TokenComponentAbstraction::ABSTRACTION_VIRTUAL;
	case ABSTRACTION_PURE_VIRTUAL:
		return TokenComponentAbstraction::ABSTRACTION_PURE_VIRTUAL;
	case ABSTRACTION_NONE:
		return TokenComponentAbstraction::ABSTRACTION_NONE;
	}
}

TokenComponentAbstraction* Storage::addAbstraction(Node* node, ParserClient::AbstractionType abstraction)
{
	if (abstraction != ABSTRACTION_NONE)
	{
		std::shared_ptr<TokenComponentAbstraction> ptr =
			std::make_shared<TokenComponentAbstraction>(convertAbstractionType(abstraction));
		node->addComponentAbstraction(ptr);
		return ptr.get();
	}
	return nullptr;
}

Edge* Storage::addTypeEdge(Node* node, Edge::EdgeType edgeType, const DataType& type)
{
	Node* typeNode = m_graph.createNodeHierarchy(type.getRawTypeName());
	Edge* edge = m_graph.createEdge(edgeType, node, typeNode);

	// FIXME: When a function uses the same type multiple times then we still only use one edge to save this,
	// but we can't store multiple DataTypes on this edge at the moment.
	if (!edge->getComponent<TokenComponentDataType>())
	{
		edge->addComponentDataType(
			std::make_shared<TokenComponentDataType>(type.getQualifierList(), type.getModifierStack())
		);
	}

	return edge;
}

Edge* Storage::addTypeEdge(Node* node, Edge::EdgeType edgeType, const ParseTypeUsage& typeUsage)
{
	if (!typeUsage.location.isValid())
	{
		return nullptr;
	}

	Edge* edge = addTypeEdge(node, edgeType, typeUsage.dataType);

	addTokenLocation(edge, typeUsage.location);
	return edge;
}

TokenLocation* Storage::addTokenLocation(Token* token, const ParseLocation& loc, bool isScope)
{
	if (!loc.isValid())
	{
		return nullptr;
	}

	TokenLocation* location = m_locationCollection.addTokenLocation(
		token->getId(), loc.filePath,
		loc.startLineNumber, loc.startColumnNumber,
		loc.endLineNumber, loc.endColumnNumber
	);

	if (isScope)
	{
		location->setType(TokenLocation::LOCATION_SCOPE);
	}

	token->addLocationId(location->getId());
	return location;
}

void Storage::log(std::string type, std::string str, const ParseLocation& location) const
{
	LOG_INFO_STREAM(
		<< type << ": " << str << " <" << location.filePath << " "
		<< location.startLineNumber << ":" << location.startColumnNumber << " "
		<< location.endLineNumber << ":" << location.endColumnNumber << ">"
	);
}

std::vector<std::tuple<Id, Id, Id>> Storage::getEdgesOfTypeOfNode(const Id id, const Edge::EdgeType type) const
{
	std::vector<std::tuple<Id, Id, Id>> result;

	Node* node = m_graph.findNode([&](Node* node){
		return node->getId() == id;
	});

	if (node != NULL)
	{
		node->forEachEdge(
				[&result, &type](Edge* e)
				{
					if(e->getType() == type)
					{
						result.push_back(std::tuple<Id, Id, Id>(e->getFrom()->getId(), e->getTo()->getId(), e->getId()));
					}
				}
			);
	}

	return result;
}
