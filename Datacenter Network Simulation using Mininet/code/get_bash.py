
with open("dump_flows.sh", mode="w+", encoding="utf-8") as f:
    f.write("#!/bin/bash\n\n")
    for i in range(1, 21):
        f.write("sudo ovs-ofctl dump-flows s{} >flowsTable/s{}_flows_table.log\n".format(i, i))