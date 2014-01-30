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


#include "roundrobintournament.h"
#include "playerbuilder.h"

RoundRobinTournament::RoundRobinTournament(GameManager* gameManager,
					   QObject *parent)
	: Tournament(gameManager, parent),
	  m_pairNumber(0)
{
}

QString RoundRobinTournament::type() const
{
	return "round-robin";
}

QList< QPair<QString, QString> > RoundRobinTournament::getPairings() const
{
	int pCount = playerCount() + (playerCount() % 2);
	QList<int> topHalf;
	QList<int> bottomHalf;

	for (int i = 0; i < pCount / 2; i++)
		topHalf.append(i);
	for (int i = pCount - 1; i >= pCount / 2; i--)
		bottomHalf.append(i);

	int pairNumber = 0;
	int currentRound = 1;
	int count = finalGameCount();
	int gameNumber = 0;
	QList< QPair<QString, QString> > pList;

	while (gameNumber < count) {
		if (pairNumber >= topHalf.size()) {
			pairNumber = 0;
			currentRound++;
			topHalf.insert(1, bottomHalf.takeFirst());
			bottomHalf.append(topHalf.takeLast());
		}
		int white = topHalf.at(pairNumber);
		int black = bottomHalf.at(pairNumber);

		if (currentRound % 2 == 0)
			qSwap(white, black);

		pairNumber++;

		if (white < playerCount() && black < playerCount()) {
			pList.append(qMakePair(playerAt(white).builder->name(), playerAt(black).builder->name()));
			gameNumber++;
		}
		else
			; // don't count byes
	}
	return pList;
}

void RoundRobinTournament::initializePairing()
{
	m_pairNumber = 0;
	m_topHalf.clear();
	m_bottomHalf.clear();
	int count = playerCount() + (playerCount() % 2);

	for (int i = 0; i < count / 2; i++)
		m_topHalf.append(i);
	for (int i = count - 1; i >= count / 2; i--)
		m_bottomHalf.append(i);
}

int RoundRobinTournament::gamesPerCycle() const
{
	return (playerCount() * (playerCount() - 1)) / 2;
}

QPair<int, int> RoundRobinTournament::nextPair()
{
	if (m_pairNumber >= m_topHalf.size())
	{
		m_pairNumber = 0;
		setCurrentRound(currentRound() + 1);
		m_topHalf.insert(1, m_bottomHalf.takeFirst());
		m_bottomHalf.append(m_topHalf.takeLast());
	}

	int white = m_topHalf.at(m_pairNumber);
	int black = m_bottomHalf.at(m_pairNumber);

	// Alternate colors between rounds to make it fair
	if (currentRound() % 2 == 0)
		qSwap(white, black);

	m_pairNumber++;

	// If 'white' or 'black' equals 'playerCount()' it means
	// that it's a "bye" player, that is an empty player that
	// makes the pairings easier to organize. In that case
	// no game is played and we skip to the next pair.
	if (white < playerCount() && black < playerCount())
		return qMakePair(white, black);
	else
		return nextPair();
}
