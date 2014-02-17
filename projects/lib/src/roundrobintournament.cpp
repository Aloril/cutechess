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
	  m_pairNumber(0),
	  m_bergerPtr(0)
{
}

QString RoundRobinTournament::type() const
{
	return "round-robin";
}

QList< QPair<QString, QString> > RoundRobinTournament::getPairings()
{
	int count = playerCount() + (playerCount() % 2);
	int roundsPerCycle = gamesPerCycle() / (count / 2);
	QList<int> bergerTable;
	QMap<QString, QPair<int, int> > WBmap;
	QString ls;
	bool wantsDebug = false;

	initializePairing(bergerTable);

	if (wantsDebug) {
		ls.clear();
		for(int i = 0; i < bergerTable.size(); i++) {
			if (i) ls += " ";
			ls += QString::number(bergerTable[i]);
		}
		qDebug("%s", qPrintable(ls));
	}

	int bergerPtr = 0;
	int gameNumber = 0;
	int currentRound = 1;
	QList< QPair<QString, QString> > pList;

	if (wantsDebug)
		qDebug("Round %d", currentRound);

	while(gameNumber < finalGameCount()) {
		if (bergerPtr >= bergerTable.size()) {
			for (int i = 0; i < count; i++) {
				if (bergerTable[i] == count - 1) ;
				else bergerTable[i] = (bergerTable[i] + (count / 2)) % (count - 1);
			}
			bergerPtr = 0;
			currentRound++;
			bergerTable.insert(!(((currentRound - 1) % roundsPerCycle) % 2), bergerTable.takeAt(bergerTable.indexOf(count - 1)));
			if (wantsDebug) {
				qDebug("Round %d", currentRound);
				ls.clear();
				for(int i = 0; i < bergerTable.size(); i++) {
					if (i) ls += " ";
					ls += QString::number(bergerTable[i]);
				}
				qDebug("%s", qPrintable(ls));
			}
		}
		int white = bergerTable[bergerPtr++];
		int black = bergerTable[bergerPtr++];

		if ((gameNumber / gamesPerCycle()) % 2)
			qSwap(white, black);

		if (white < playerCount() && black < playerCount()) {
			pList.append(qMakePair(playerAt(white).builder->name(), playerAt(black).builder->name()));

			if (wantsDebug) {
				int wW, wB;
				int bW, bB;
				QPair<int, int>& thisPair = WBmap[playerAt(white).builder->name()];
				thisPair.first++;
				wW = thisPair.first;
				wB = thisPair.second;
				QPair<int, int>& thatPair = WBmap[playerAt(black).builder->name()];
				thatPair.second++;
				bW = thatPair.first;
				bB = thatPair.second;
				qDebug("%s (%dW %dB) - %s (%dW %dB)", qPrintable(playerAt(white).builder->name()), wW, wB, qPrintable(playerAt(black).builder->name()), bW, bB);
			}
			gameNumber++;
		} else {
			// what about byes? cutechess doesn't usually count them
		}
	}
	//exit(0);
	return pList;
}

void RoundRobinTournament::initializePairing()
{
	m_pairNumber = 0;
	m_bergerPtr = 0;
	initializePairing(m_bergerTable);
}

void RoundRobinTournament::initializePairing(QList<int>& bergerTable)
{
	int count = playerCount() + (playerCount() % 2);

	bergerTable.clear();
	bergerTable.reserve(count);
	for (int i  = 0; i < count; i++)
		bergerTable.append(0);
	for (int i = 0; i < count / 2; i++)
		bergerTable[i * 2] = i;
	for (int i = count - 1; i >= count / 2; i--)
		bergerTable[((count - i) * 2) - 1] = i;
}

int RoundRobinTournament::gamesPerCycle() const
{
	return (playerCount() * (playerCount() - 1)) / 2;
}

QPair<int, int> RoundRobinTournament::nextPair()
{
	int count = playerCount() + (playerCount() % 2);
	int roundsPerCycle = gamesPerCycle() / (count / 2);

	if (m_bergerPtr >= m_bergerTable.size()) {
		for (int i = 0; i < count; i++) {
			if (m_bergerTable[i] == count - 1) ;
			else m_bergerTable[i] = (m_bergerTable[i] + (count / 2)) % (count - 1);
		}
		m_bergerPtr = 0;
		setCurrentRound(currentRound() + 1);
		m_bergerTable.insert(!(((currentRound() - 1) % roundsPerCycle) % 2), m_bergerTable.takeAt(m_bergerTable.indexOf(count - 1)));
	}

	int white = m_bergerTable[m_bergerPtr++];
	int black = m_bergerTable[m_bergerPtr++];

	if ((m_pairNumber / gamesPerCycle()) % 2)
		qSwap(white, black);

	// If 'white' or 'black' equals 'playerCount()' it means
	// that it's a "bye" player, that is an empty player that
	// makes the pairings easier to organize. In that case
	// no game is played and we skip to the next pair.
	if (white < playerCount() && black < playerCount()) {
		m_pairNumber++;
		return qMakePair(white, black);
	} else
		return nextPair();
}
