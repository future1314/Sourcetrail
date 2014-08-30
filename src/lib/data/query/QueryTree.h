#ifndef QUERY_TREE_H
#define QUERY_TREE_H

#include <deque>
#include <memory>
#include <ostream>
#include <string>

#include "data/query/QueryOperator.h"

class QueryNode;

class QueryTree
{
public:
	QueryTree(std::string query);
	~QueryTree();

	std::shared_ptr<QueryNode> getRoot() const;

	bool isValid() const;

	void print(std::ostream& ostream) const;

private:
	std::shared_ptr<QueryNode> buildTree(std::deque<std::string>& tokens, std::shared_ptr<QueryNode> frontNode);
	std::shared_ptr<QueryNode> buildGroup(std::deque<std::string>& tokens, QueryOperator::OperatorType closeType);
	std::shared_ptr<QueryNode> getNextNode(std::deque<std::string>& tokens);
	std::shared_ptr<QueryNode> createCommand(std::string name);

	std::shared_ptr<QueryNode> m_root;
	std::string m_query;
	bool m_valid;
};

std::ostream& operator<<(std::ostream& ostream, const QueryTree& tree);

#endif // QUERY_TREE_H
