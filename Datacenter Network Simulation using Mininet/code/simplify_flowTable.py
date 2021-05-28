def simplify(filename):
    with open("../flowsTable/WithController/origin/"+filename+".log", mode="r", encoding="utf-8") as sf:
        with open("../flowsTable/WithController/simple/"+filename+"_simple.log", mode="w", encoding="utf-8") as outf:
            lines = sf.readlines()
            srcs = []
            dsts = []
            ping_count = 0
            for j, line in enumerate(lines):
                parts = line.split(',')
                # if line[:9] == "NXST_FLOW":
                #     ping_count += 1
                # if ping_count < 3:
                #     continue
                if len(parts) < 14:
                    continue
                # 已有的规则
                if parts[11] in srcs and parts[12] in dsts:
                    continue
                # 新规则
                new_line = ""
                for index in needed_parts:
                    new_line += (parts[index] + ",")
                action_index = line.index("action")
                new_line += line[action_index:]
                print(new_line)
                outf.write(new_line)


if __name__ == "__main__":
    needed_parts = [7, 9, 10, 11, 12]
    for i in range(1, 21):
        name = "s{}_flows_table".format(i)
        simplify(name)
