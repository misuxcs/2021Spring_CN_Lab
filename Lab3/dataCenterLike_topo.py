from mininet.topo import Topo

K = 4
coreSwitchList = []
aggregationSwitchList = []
edgeSwitchList = []
hostList = []

class dataCenterLike(Topo):
    def __init__(self):
        #Initialize topology
        Topo.__init__(self)
        SWITCH = 's'
        SWITCH_NUM = 1
        HOST = 'h'
        HOST_NUM = 1
        #Add hosts and switches
        ## Core Layer
        coreSwitchNum = (K/2)**2
        for i  in range(coreSwitchNum):
            s_tmp = self.addSwitch(SWITCH + str(SWITCH_NUM))
            coreSwitchList.append(s_tmp)
            SWITCH_NUM += 1

        ## Aggregation Layer
        aggregationSwitchNum = K * (K/2)
        for i in range(aggregationSwitchNum):
            s_tmp = self.addSwitch(SWITCH + str(SWITCH_NUM))
            aggregationSwitchList.append(s_tmp)
            SWITCH_NUM += 1
        
        ## Edge Layer
        edgeSwitchNum = K * (K/2)
        for i in range(edgeSwitchNum):
            s_tmp = self.addSwitch(SWITCH + str(SWITCH_NUM))
            edgeSwitchList.append(s_tmp)
            SWITCH_NUM += 1
        
        ## Hosts
        hostNum = edgeSwitchNum * (K/2)
        for i in range(hostNum):
            s_tmp = self.addHost(HOST+ str(HOST_NUM))
            hostList.append(s_tmp)
            HOST_NUM += 1
        
        #Add links
        perEnds = K/2
        for i in range(edgeSwitchNum):
            for j in range(perEnds):
                self.addLink(edgeSwitchList[i], hostList[i*perEnds+j])

        for i in range(aggregationSwitchNum):
            for j in range(perEnds):
                self.addLink(aggregationSwitchList[i], edgeSwitchList[(i/perEnds)*perEnds+j])

        for i in range(aggregationSwitchNum):
            for j in range(perEnds):
                for k in range(perEnds):
                    if (i+j) < 8:
                        self.addLink(coreSwitchList[j*perEnds+k], aggregationSwitchList[i+j])

topos = {'dataCenterLike':(lambda: dataCenterLike() )}