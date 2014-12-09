# -*- coding: UTF-8 -*-

from logic import basic, map_info, gamebody

STATE_CONTINUE = -1
STATE_TIE = 2

#TEST_LEVEL
TEST_PRODUCE = 0
TEST_COLLECT = 1
TEST_CAPTURE = 2
TEST_ATTACK_UNIT = 3
TEST_REAL_GAME = 4


def testgame_state(game, events, test_level):
    if test_level == TEST_REAL_GAME:
        dead_bases = 0
        rst = None
        for element in game.map_info.elements.values():
            if element.kind == basic.BASE and element.health <= 0:
                dead_bases += 1
                rst = 1 - element.team
        if dead_bases == 1:
            return rst
        if game.round >= game.max_round or dead_bases == 2:
            return 0 if game.score(0) > game.score(1) else (1 if game.score(1) > game.score(0) else STATE_TIE)
        return STATE_CONTINUE
    elif test_level == TEST_PRODUCE:
        test_state = -1
        for event in events:
            if event.__class__.__name__ == 'Create':
                test_state = 2 if test_state == 1 - game.map_info.elements[event.index].team or test_state == 2 \
                    else game.map_info.elements[event.index].team
        if game.test_state >= 0:
            if game.test_state in [0, 1]:
                return game.test_state
            else:
                return STATE_TIE
        if game.round >= game.max_round:
            return 0 if game.score(0) > game.score(1) else (1 if game.score(1) > game.score(0) else STATE_TIE)
        return STATE_CONTINUE 
    elif test_level == TEST_COLLECT:
        for element in game.map_info.elements.values():
            if game.scores[element.team] > 0 and element.kind == basic.BASE and \
                            element.fuel == basic.PROPERTY[basic.BASE]['fuel_max'] and \
                            element.metal == basic.PROPERTY[basic.BASE]['metal_max']:
                return element.team
        if game.round >= game.max_round:
            return STATE_TIE
        return STATE_CONTINUE
    elif test_level == TEST_CAPTURE:
        for element in game.map_info.elements.values():
            if game.scores[element.team] > 0 and element.kind == basic.FORT and element.team == game.team:
                return element.team
        if game.round >= game.max_round:
            return STATE_TIE
        return STATE_CONTINUE
    elif test_level == TEST_ATTACK_UNIT:
        test_state = -1
        for event in events:
            if event.__class__.__name__ == 'AttackUnit' and not event.taregt in game.map_info.elements:
                test_state = 2 if test_state == 1 - game.map_info.elements[event.index].team or test_state == 2 \
                    else game.map_info.elements[event.index].team
        if game.test_state >= 0:
            if game.test_state in [0, 1]:
                return game.test_state
            else:
                return STATE_TIE
        if game.round >= game.max_round:
            return 0 if game.score(0) > game.score(1) else (1 if game.score(1) > game.score(0) else STATE_TIE)
        return STATE_CONTINUE 
    else:
        if game.round >= game.max_round:
            return STATE_TIE
        return STATE_CONTINUE