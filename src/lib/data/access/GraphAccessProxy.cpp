#include "data/access/GraphAccessProxy.h"

#include "utility/logging/logging.h"

GraphAccessProxy::GraphAccessProxy()
	: m_subject(nullptr)
{
}

GraphAccessProxy::~GraphAccessProxy()
{
}

bool GraphAccessProxy::hasSubject() const
{
	if (m_subject)
	{
		return true;
	}

	LOG_ERROR("GraphAccessProxy has no subject.");
	return false;
}

void GraphAccessProxy::setSubject(GraphAccess* subject)
{
	m_subject = subject;
}

Id GraphAccessProxy::getIdForNodeWithName(const std::string& name) const
{
	if (hasSubject())
	{
		return m_subject->getIdForNodeWithName(name);
	}

	return 0;
}

std::string GraphAccessProxy::getNameForNodeWithId(Id id) const
{
	if (hasSubject())
	{
		return m_subject->getNameForNodeWithId(id);
	}

	return "";
}

std::vector<std::string> GraphAccessProxy::getNamesForNodesWithNamePrefix(const std::string& prefix) const
{
	if (hasSubject())
	{
		return m_subject->getNamesForNodesWithNamePrefix(prefix);
	}

	return std::vector<std::string>();
}

std::vector<Id> GraphAccessProxy::getIdsOfNeighbours(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->getIdsOfNeighbours(id);
	}

	return std::vector<Id>();
}

std::vector<std::tuple<Id, Id, Id>> GraphAccessProxy::getNeighbourEdgesOfNode(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->getNeighbourEdgesOfNode(id);
	}

	return std::vector<std::tuple<Id, Id, Id>>();
}

std::vector<std::tuple<Id, Id, Id>> GraphAccessProxy::getMemberEdgesOfNode(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->getMemberEdgesOfNode(id);
	}

	return std::vector<std::tuple<Id, Id, Id>>();
}

std::vector<std::tuple<Id, Id, Id>> GraphAccessProxy::getUsageEdgesOfNode(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->getUsageEdgesOfNode(id);
	}

	return std::vector<std::tuple<Id, Id, Id>>();
}

std::vector<std::tuple<Id, Id, Id>> GraphAccessProxy::getCallEdgesOfNode(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->getCallEdgesOfNode(id);
	}

	return std::vector<std::tuple<Id, Id, Id>>();
}

std::vector<std::tuple<Id, Id, Id>> GraphAccessProxy::getTypeOfEdgesOfNode(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->getTypeOfEdgesOfNode(id);
	}

	return std::vector<std::tuple<Id, Id, Id>>();
}

std::vector<std::tuple<Id, Id, Id>> GraphAccessProxy::getReturnTypeOfEdgesOfNode(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->getReturnTypeOfEdgesOfNode(id);
	}

	return std::vector<std::tuple<Id, Id, Id>>();
}

std::vector<std::tuple<Id, Id, Id>> GraphAccessProxy::getParameterOfEdgesOfNode(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->getParameterOfEdgesOfNode(id);
	}

	return std::vector<std::tuple<Id, Id, Id>>();
}

std::vector<std::tuple<Id, Id, Id>> GraphAccessProxy::getInheritanceEdgesOfNode(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->getInheritanceEdgesOfNode(id);
	}

	return std::vector<std::tuple<Id, Id, Id>>();
}

std::pair<Id, Id> GraphAccessProxy::getNodesOfEdge(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->getNodesOfEdge(id);
	}

	return std::pair<Id, Id>();
}

bool GraphAccessProxy::checkTokenIsNode(const Id id) const
{
	if (hasSubject())
	{
		return m_subject->checkTokenIsNode(id);
	}

	return false;
}

std::vector<Id> GraphAccessProxy::getActiveTokenIdsForId(Id tokenId) const
{
	if (hasSubject())
	{
		return m_subject->getActiveTokenIdsForId(tokenId);
	}

	return std::vector<Id>();
}

std::vector<Id> GraphAccessProxy::getLocationIdsForTokenIds(const std::vector<Id>& tokenIds) const
{
	if (hasSubject())
	{
		return m_subject->getLocationIdsForTokenIds(tokenIds);
	}

	return std::vector<Id>();
}

std::vector<Id> GraphAccessProxy::getTokenIdsForQuery(std::string query) const
{
	if (hasSubject())
	{
		return m_subject->getTokenIdsForQuery(query);
	}

	return std::vector<Id>();
}
