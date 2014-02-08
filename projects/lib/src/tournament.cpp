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


#include "tournament.h"
#include <QFile>
#include "gamemanager.h"
#include "playerbuilder.h"
#include "board/boardfactory.h"
#include "chessplayer.h"
#include "chessgame.h"
#include "pgnstream.h"
#include "openingsuite.h"
#include "sprt.h"

Tournament::Tournament(GameManager* gameManager, QObject *parent)
	: QObject(parent),
	  m_gameManager(gameManager),
	  m_lastGame(0),
	  m_variant("standard"),
	  m_round(0),
	  m_nextGameNumber(0),
	  m_finishedGameCount(0),
	  m_savedGameCount(0),
	  m_finalGameCount(0),
	  m_gamesPerEncounter(1),
	  m_roundMultiplier(1),
	  m_startDelay(0),
	  m_openingDepth(1024),
	  m_stopping(false),
	  m_repeatOpening(false),
	  m_recover(false),
	  m_pgnCleanup(true),
	  m_finished(false),
	  m_openingSuite(0),
	  m_sprt(new Sprt),
	  m_pgnOutMode(PgnGame::Verbose),
	  m_pair(QPair<int, int>(-1, -1)),
	  m_livePgnOutMode(PgnGame::Verbose),
	  m_resumeGameNumber(0)
{
	Q_ASSERT(gameManager != 0);
}

Tournament::~Tournament()
{
	if (!m_gameData.isEmpty())
		qWarning("Tournament: Destroyed while games are still running.");

	qDeleteAll(m_gameData);
	foreach (const PlayerData& data, m_players)
		delete data.builder;

	delete m_openingSuite;
	delete m_sprt;
}

GameManager* Tournament::gameManager() const
{
	return m_gameManager;
}

bool Tournament::isFinished() const
{
	return m_finished;
}

QString Tournament::errorString() const
{
	return m_error;
}

QString Tournament::name() const
{
	return m_name;
}

QString Tournament::site() const
{
	return m_site;
}

QString Tournament::variant() const
{
	return m_variant;
}

int Tournament::currentRound() const
{
	return m_round;
}

int Tournament::gamesPerEncounter() const
{
	return m_gamesPerEncounter;
}

int Tournament::roundMultiplier() const
{
	return m_roundMultiplier;
}

int Tournament::finishedGameCount() const
{
	return m_finishedGameCount;
}

int Tournament::finalGameCount() const
{
	return m_finalGameCount;
}

Tournament::PlayerData Tournament::playerAt(int index) const
{
	return m_players.at(index);
}

int Tournament::playerCount() const
{
	return m_players.size();
}

Sprt* Tournament::sprt() const
{
	return m_sprt;
}

void Tournament::setName(const QString& name)
{
	m_name = name;
}

void Tournament::setSite(const QString& site)
{
	m_site = site;
}

void Tournament::setEventDate(const QString& eventDate)
{
	m_eventDate = eventDate;
}

void Tournament::setVariant(const QString& variant)
{
	Q_ASSERT(Chess::BoardFactory::variants().contains(variant));
	m_variant = variant;
}

void Tournament::setCurrentRound(int round)
{
	Q_ASSERT(round >= 1);
	m_round = round;
}

void Tournament::setGamesPerEncounter(int count)
{
	Q_ASSERT(count > 0);
	m_gamesPerEncounter = count;
}

void Tournament::setRoundMultiplier(int factor)
{
	Q_ASSERT(factor > 0);
	m_roundMultiplier = factor;
}

void Tournament::setStartDelay(int delay)
{
	Q_ASSERT(delay >= 0);
	m_startDelay = delay;
}

void Tournament::setRecoveryMode(bool recover)
{
	m_recover = recover;
}

void Tournament::setAdjudicator(const GameAdjudicator& adjudicator)
{
	m_adjudicator = adjudicator;
}

void Tournament::setOpeningSuite(OpeningSuite *suite)
{
	delete m_openingSuite;
	m_openingSuite = suite;
}

void Tournament::setOpeningDepth(int plies)
{
	m_openingDepth = plies;
}

void Tournament::setPgnOutput(const QString& fileName, PgnGame::PgnMode mode)
{
	m_pgnout = fileName;
	m_pgnOutMode = mode;
}

void Tournament::setLivePgnOutput(const QString& fileName, PgnGame::PgnMode mode)
{
	m_livePgnout = fileName;
	m_livePgnOutMode = mode;
}

void Tournament::setPgnCleanupEnabled(bool enabled)
{
	m_pgnCleanup = enabled;
}

void Tournament::setOpeningRepetition(bool repeat)
{
	m_repeatOpening = repeat;
}

void Tournament::setResume(int nextGameNumber)
{
	Q_ASSERT(nextGame >= 0);
	m_resumeGameNumber = nextGameNumber;
}

void Tournament::addPlayer(PlayerBuilder* builder,
			   const TimeControl& timeControl,
			   const OpeningBook* book,
			   int bookDepth)
{
	Q_ASSERT(builder != 0);

	PlayerData data = { builder, timeControl, book, bookDepth, 0, 0, 0 };
	m_players.append(data);
}

ChessGame* Tournament::setupBoard(PlayerData& white, PlayerData& black)
{
	Chess::Board* board = Chess::BoardFactory::create(m_variant);
	Q_ASSERT(board != 0);
	ChessGame* game = new ChessGame(board, new PgnGame());

	game->setTimeControl(white.timeControl, Chess::Side::White);
	game->setTimeControl(black.timeControl, Chess::Side::Black);

	game->setOpeningBook(white.book, Chess::Side::White, white.bookDepth);
	game->setOpeningBook(black.book, Chess::Side::Black, black.bookDepth);

	QString whiteName = white.builder->name();
	QString blackName = black.builder->name();

	bool isRepeat = false;
	if (m_openingHistory.contains(blackName)) {
		QVariantMap mMap = m_openingHistory[blackName].toMap();
		if (mMap.contains(whiteName)) {
			QVariantMap mmMap = mMap[whiteName].toMap();
			QString fenString = mmMap["fenString"].toString();
			QVariantList mList = mmMap["moves"].toList();

			QVector<Chess::Move> moves;
			QVariantList::const_iterator mv;
			for (mv = mList.begin(); mv != mList.end(); ++mv) {
				QVariantList mvList = mv->toList();
				Chess::Move move(mvList.at(0).toInt(), mvList.at(1).toInt());
				moves.append(move);
			}
			// qDebug("got %s in %s's history (%d moves)", qPrintable(whiteName), qPrintable(blackName), moves.size());
			game->setStartingFen(fenString);
			game->setMoves(moves);
			mMap.remove(whiteName); // get it out of there so we don't find it again
			m_openingHistory.insert(blackName, mMap);
			isRepeat = true;
		}
	}
	if (m_openingSuite && !isRepeat)
		game->setMoves(m_openingSuite->nextGame(m_openingDepth));

	game->generateOpening();

	// save the opening moves in the history
	if (m_repeatOpening && !isRepeat) {
		QString fenString = game->startingFen();
		QVector<Chess::Move> moves = game->moves();
		QVector<Chess::Move>::const_iterator mv;
		QVariantList mList;
		for (mv = moves.begin(); mv != moves.end(); ++mv) {
			QVariantList mvList;
			mvList.append(mv->sourceSquare());
			mvList.append(mv->targetSquare());
			mList.append(QVariant(mvList));
		}
		QVariantMap mMap = m_openingHistory[whiteName].toMap();
		QVariantMap mmMap;
		mmMap.insert("fenString", fenString);
		mmMap.insert("moves", mList);
		mMap.insert(blackName, mmMap);
		m_openingHistory.insert(whiteName, mMap);
	}
	return game;
}

void Tournament::startNextGame()
{
	if (m_stopping || m_nextGameNumber >= m_finalGameCount)
		return;

	m_pair = nextPair();

	PlayerData& white = m_players[m_pair.first];
	PlayerData& black = m_players[m_pair.second];

	ChessGame* game = setupBoard(white, black);

	connect(game, SIGNAL(started(ChessGame*)),
		this, SLOT(onGameStarted(ChessGame*)));
	connect(game, SIGNAL(finished(ChessGame*)),
		this, SLOT(onGameFinished(ChessGame*)));
	connect(game, SIGNAL(pgnMove()),
		this, SLOT(onPgnMove()));

	game->pgn()->setEvent(m_name);
	game->pgn()->setSite(m_site);
	game->pgn()->setRound(m_round);
	if (!m_eventDate.isEmpty())
		game->pgn()->setEventDate(m_eventDate);

	game->setStartDelay(m_startDelay);
	game->setAdjudicator(m_adjudicator);

	GameData* data = new GameData;
	data->number = ++m_nextGameNumber;
	data->whiteIndex = m_pair.first;
	data->blackIndex = m_pair.second;
	m_gameData[game] = data;

	connect(game, SIGNAL(startFailed(ChessGame*)),
		this, SLOT(onGameStartFailed(ChessGame*)));
	m_gameManager->newGame(game,
			       white.builder,
			       black.builder,
			       GameManager::Enqueue,
			       GameManager::ReusePlayers);
}

void Tournament::onGameStarted(ChessGame* game)
{
	Q_ASSERT(game != 0);
	Q_ASSERT(m_gameData.contains(game));

	GameData* data = m_gameData[game];
	m_players[data->whiteIndex].builder->setName(game->player(Chess::Side::White)->name());
	m_players[data->blackIndex].builder->setName(game->player(Chess::Side::Black)->name());

	emit gameStarted(game, data->number, data->whiteIndex, data->blackIndex);
}

void Tournament::onPgnMove()
{
	if (m_livePgnout.isEmpty()) return;

	ChessGame* sender = qobject_cast<ChessGame*>(QObject::sender());
	Q_ASSERT(sender != 0);

	PgnGame* pgn(sender->pgn());

	QFile::resize(m_livePgnout, 0);
	pgn->write(m_livePgnout, m_livePgnOutMode);
}

void Tournament::onGameFinished(ChessGame* game)
{
	Q_ASSERT(game != 0);

	PgnGame* pgn(game->pgn());
	Chess::Result result(game->result());

	m_finishedGameCount++;

	Q_ASSERT(m_gameData.contains(game));
	GameData* data = m_gameData.take(game);
	int gameNumber = data->number;
	Sprt::GameResult sprtResult = Sprt::NoResult;

	switch (game->result().winner())
	{
	case Chess::Side::White:
		m_players[data->whiteIndex].wins++;
		m_players[data->blackIndex].losses++;
		sprtResult = (data->whiteIndex == 0) ? Sprt::Win : Sprt::Loss;
		break;
	case Chess::Side::Black:
		m_players[data->blackIndex].wins++;
		m_players[data->whiteIndex].losses++;
		sprtResult = (data->blackIndex == 0) ? Sprt::Win : Sprt::Loss;
		break;
	default:
		if (game->result().isDraw())
		{
			m_players[data->whiteIndex].draws++;
			m_players[data->blackIndex].draws++;
			sprtResult = Sprt::Draw;
		}
		break;
	}

	if (!m_pgnout.isEmpty())
	{
		m_pgnGames[gameNumber] = *pgn;

		if (!m_livePgnout.isEmpty()) {
			QFile::resize(m_livePgnout, 0);
			pgn->write(m_livePgnout, m_livePgnOutMode); // write the final pgn
		}

		while (m_pgnGames.contains(m_savedGameCount + 1))
		{
			PgnGame tmp = m_pgnGames.take(++m_savedGameCount);
			if (!tmp.write(m_pgnout, m_pgnOutMode))
				qWarning("Can't write to PGN file %s", qPrintable(m_pgnout));
		}
	}

	Chess::Result::Type resultType(game->result().type());
	bool crashed = (resultType == Chess::Result::Disconnection ||
			resultType == Chess::Result::StalledConnection);
	if (!m_recover && crashed)
		stop();

	if (!m_sprt->isNull() && sprtResult != Sprt::NoResult)
	{
		m_sprt->addResult(sprtResult);
		if (m_sprt->status() != Sprt::Continue)
			QMetaObject::invokeMethod(this, "stop", Qt::QueuedConnection);
	}

	emit gameFinished(game, gameNumber, data->whiteIndex, data->blackIndex);

	if (m_finishedGameCount == m_finalGameCount
	||  (m_stopping && m_gameData.isEmpty()))
	{
		m_stopping = false;
		m_lastGame = game;
		connect(m_gameManager, SIGNAL(gameDestroyed(ChessGame*)),
			this, SLOT(onGameDestroyed(ChessGame*)));
	}

	if (m_pgnCleanup)
		delete pgn;

	delete data;
	game->deleteLater();
}

void Tournament::onGameDestroyed(ChessGame* game)
{
	if (game != m_lastGame)
		return;

	m_lastGame = 0;
	m_gameManager->cleanupIdleThreads();
	m_finished = true;
	emit finished();
}

void Tournament::onGameStartFailed(ChessGame* game)
{
	m_error = game->errorString();

	delete game->pgn();
	game->deleteLater();
	m_gameData.remove(game);

	stop();
}

void Tournament::start()
{
	Q_ASSERT(m_players.size() > 1);

	m_round = 1;
	m_nextGameNumber = 0;
	m_finishedGameCount = 0;
	m_savedGameCount = 0;
	m_finalGameCount = 0;
	m_stopping = false;

	m_gameData.clear();
	m_pgnGames.clear();
	m_openingHistory.clear();

	connect(m_gameManager, SIGNAL(ready()),
		this, SLOT(startNextGame()));

	initializePairing();
	m_finalGameCount = gamesPerCycle() * gamesPerEncounter() * roundMultiplier();

	if (m_resumeGameNumber) {
		int nextGame = m_resumeGameNumber;
		OpeningSuite* pgngames = NULL;
		if (!m_pgnout.isEmpty()) {
			pgngames = new OpeningSuite(m_pgnout, OpeningSuite::PgnFormat, OpeningSuite::SequentialOrder, 0);
		}

		while (nextGame--) {
			if (m_nextGameNumber >= m_finalGameCount)
				return;

			m_pair = nextPair();

			if (m_openingSuite != 0) {
				PlayerData& white = m_players[m_pair.first];
				PlayerData& black = m_players[m_pair.second];

				ChessGame* game = setupBoard(white, black);
				delete game;
			}

			if (pgngames) {
				m_pgnGames[++m_savedGameCount] = pgngames->nextGame(INT_MAX - 1);
			}

			++m_nextGameNumber;
			++m_finishedGameCount;
		}

		if (pgngames)
			delete pgngames;
	}
	startNextGame();
}

void Tournament::stop()
{
	if (m_stopping)
		return;

	disconnect(m_gameManager, SIGNAL(ready()),
		   this, SLOT(startNextGame()));

	if (m_gameData.isEmpty())
	{
		m_gameManager->cleanupIdleThreads();
		m_finished = true;
		emit finished();
		return;
	}

	m_stopping = true;
	foreach (ChessGame* game, m_gameData.keys())
		QMetaObject::invokeMethod(game, "stop", Qt::QueuedConnection);
}
