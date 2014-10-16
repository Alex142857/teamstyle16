import json
import logging
import socket
import time

from logic import gamebody
import battle
import ai_proxy

logger = logging.getLogger(__name__)

class AIBattle(battle.Battle):
    """Represent a battle between two AIs"""
    def __init__(self, map_info, port=ai_proxy.DEFAULT_PORT,
                 ai0_filename=None, ai1_filename=None, prev_info=None):
        """Construct an AIBattle from a map, or from previous infos.
        port is the port number used for listening.
        If filename is given, AIBattle is responsible for starting & closing
        the AI program.
        If prev_info is given, restore from it instead of constructing a new
        one.
        """
        # Start battle
        super(AIBattle, self).__init__(map_info, prev_info=prev_info)
        # build the socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while True:
            try:
                sock.bind(('', port))
            except socket.error:
                logger.warning('Port %d has already been taken, trying port %d',
                               port, port + 1)
                port += 1
            else:  # binding succeeds
                break
        sock.listen(2)

        logger.debug('Building proxies for AIs')
        self.ais = []
        self.ais.append(ai_proxy.AIProxy(0, sock=sock, battle=self,
                                            filename=ai0_filename))
        self.ais.append(ai_proxy.AIProxy(1, sock=sock, battle=self,
                                            filename=ai1_filename))
        logger.info('Proxies for AIs built')

        # Set team names in Battle
        self.team_names = [self.ais[0].team_name, self.ais[1].team_name]

        # Battle has been started, so send infos to AIs
        for ai in self.ais:
            ai.send_info(self)

        for ai in self.ais:
            ai.start()

    def __del__(self):
        for ai in self.ais:
            logger.debug('Stopping AI %d proxy', ai.team_num)
            ai.stop()
            ai.join()
            logger.info('AI %d proxy stopped', ai.team_num)

    # Override team_name(), to make sure team names of AI will be displayed,
    # instead of team names set at the beginning of the battle.
    # e.g. the battle may be started as a HumanAIBattle, but later taken over
    # as an AIBattle
    def team_name(self, team):
        return self.ais[team].team_name

    def next_round(self):
        """Advance the game to the next round, return events happened"""
        events = battle.Battle.next_round(self)
        for ai in self.ais:
            ai.send_info(self)
        return events

    def feed_ai_commands(self, sleep_time=None):
        """Feed commands from AIs within time limit time_per_round"""
        logger.debug('Feeding commands')
        if sleep_time is None:
            sleep_time = self.gamebody.time_per_round
        time.sleep(sleep_time)

        cmds = []
        # Collect commands for both teams first, or team 1 would have more
        # time to send commands
        for ai in self.ais:
            cmds.append(ai.get_commands())

        for ai in self.ais:
            for cmd in cmds:
                self.gamebody.set_command(ai.team_num, cmd)
        logger.info('AI commands feed')

    def run_until_end(self):
        """Run until the game ends"""
        while self.gamebody.state() == gamebody.STATE_CONTINUE:
            self.feed_ai_commands()
            self.next_round()

def load(filename, port=ai_proxy.DEFAULT_PORT, ai0_filename=None, ai1_filename=None):
    """Load save file, construct an AIBattle based on it"""
    return AIBattle(None, port=port,
                    ai0_filename=ai0_filename, ai1_filename=ai1_filename,
                    prev_info=json.load(open(filename)))
