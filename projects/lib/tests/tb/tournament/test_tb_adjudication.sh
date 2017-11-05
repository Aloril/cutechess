#!/bin/bash
BASENAME=test_tb_syzygy5_arg
./cutechess-cli -openings file=tb_adjudication_all.pgn -tb /usr/games/syzygy -tbpieces 5 -debug -engine name="Stockfish 8A" cmd=stockfish_8 proto=uci option.SyzygyPath=/usr/games/syzygy option."Move Overhead"=100 option.Hash=128 tc=10+0.1 -engine name="Stockfish 8B" cmd=stockfish_8 proto=uci option.SyzygyPath=/usr/games/syzygy option."Move Overhead"=100 option.Hash=128 tc=10+0.1 -games 1 -rounds 7 -pgnout ${BASENAME}.pgn > ${BASENAME}.log

BASENAME=test_tb_syzygy6_arg
./cutechess-cli -openings file=tb_adjudication_all.pgn -tb /usr/games/syzygy -tbpieces 6 -debug -engine name="Stockfish 8A" cmd=stockfish_8 proto=uci option.SyzygyPath=/usr/games/syzygy option."Move Overhead"=100 option.Hash=128 tc=10+0.1 -engine name="Stockfish 8B" cmd=stockfish_8 proto=uci option.SyzygyPath=/usr/games/syzygy option."Move Overhead"=100 option.Hash=128 tc=10+0.1 -games 1 -rounds 7 -pgnout ${BASENAME}.pgn > ${BASENAME}.log

BASENAME=test_tb_syzygy5_ignore50_arg
./cutechess-cli -openings file=tb_adjudication_all.pgn -tb /usr/games/syzygy -tbpieces 5 -tbignore50 -debug -engine name="Stockfish 8A" cmd=stockfish_8 proto=uci option.SyzygyPath=/usr/games/syzygy option."Move Overhead"=100 option.Hash=128 tc=10+0.1 -engine name="Stockfish 8B" cmd=stockfish_8 proto=uci option.SyzygyPath=/usr/games/syzygy option."Move Overhead"=100 option.Hash=128 tc=10+0.1 -games 1 -rounds 7 -pgnout ${BASENAME}.pgn > ${BASENAME}.log

BASENAME=test_tb_syzygy6_ignore50_arg
./cutechess-cli -openings file=tb_adjudication_all.pgn -tb /usr/games/syzygy -tbpieces 6 -tbignore50 -debug -engine name="Stockfish 8A" cmd=stockfish_8 proto=uci option.SyzygyPath=/usr/games/syzygy option."Move Overhead"=100 option.Hash=128 tc=10+0.1 -engine name="Stockfish 8B" cmd=stockfish_8 proto=uci option.SyzygyPath=/usr/games/syzygy option."Move Overhead"=100 option.Hash=128 tc=10+0.1 -games 1 -rounds 7 -pgnout ${BASENAME}.pgn > ${BASENAME}.log

for BASENAME in test_tb_syzygy5 test_tb_syzygy6 test_tb_syzygy5_ignore50 test_tb_syzygy6_ignore50
do
  cp -a ${BASENAME}_input.json ${BASENAME}.json
  ./cutechess-cli -tournamentfile ${BASENAME}.json -debug -resume -pgnout ${BASENAME}.pgn > ${BASENAME}.log
done
