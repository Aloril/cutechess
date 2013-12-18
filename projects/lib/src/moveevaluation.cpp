/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "moveevaluation.h"

MoveEvaluation::MoveEvaluation()
	: m_isBookEval(false),
	  m_depth(0),
	  m_score(0),
	  m_time(0),
	  m_nodeCount(0),
	  m_nps(0),
	  m_tbHits(0)
{
}

bool MoveEvaluation::isEmpty() const
{
	if (m_depth == 0
	&&  m_score == 0
	&&  m_time < 500
	&&  m_nodeCount == 0)
		return true;
	return false;
}

bool MoveEvaluation::isBookEval() const
{
	return m_isBookEval;
}

int MoveEvaluation::depth() const
{
	return m_depth;
}

int MoveEvaluation::score() const
{
	return m_score;
}

int MoveEvaluation::time() const
{
	return m_time;
}

int MoveEvaluation::nodeCount() const
{
	return m_nodeCount;
}

QString MoveEvaluation::pv() const
{
	return m_pv;
}

int MoveEvaluation::nps() const
{
	return m_nps;
}

int MoveEvaluation::tbHits() const
{
	return m_tbHits;
}

void MoveEvaluation::clear()
{
	m_isBookEval = false;
	m_depth = 0;
	m_score = 0;
	m_time = 0;
	m_nodeCount = 0;
	m_pv.clear();
}

void MoveEvaluation::setBookEval(bool isBookEval)
{
	m_isBookEval = isBookEval;
}

void MoveEvaluation::setDepth(int depth)
{
	m_depth = depth;
}

void MoveEvaluation::setScore(int score)
{
	m_score = score;
}

void MoveEvaluation::setTime(int time)
{
	m_time = time;
}

void MoveEvaluation::setNodeCount(int nodeCount)
{
	m_nodeCount = nodeCount;
}

void MoveEvaluation::setPv(const QString& pv)
{
	m_pv = pv;
}

void MoveEvaluation::setNps(int nps)
{
	// note that this is only applicable to UCI engines
	m_nps = nps;
}

void MoveEvaluation::setTbHits(int tbHits)
{
	// note that this is only applicable to UCI engines
	m_tbHits = tbHits;
}
