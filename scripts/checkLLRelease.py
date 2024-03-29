import requests
import base64
import git
import re

BDSVersion = "0.0.0"
LLVersion = "0.0.0"
VERSION_PATH = "./LLCheckBag/Version.h"

versionFile = open(VERSION_PATH,'r+')
versionInfo = versionFile.readlines()
versionFile.close()

#获取LL协议，并存下此LL支持的BDS版本
def getLiteLoaderProtocol():
    global LLVersion
    global BDSVersion
    response = requests.get("https://api.github.com/repos/LiteLDev/LiteLoaderBDS/releases/latest")
    response.raise_for_status() #如果得到404或者502抛出错误
    LLVersion = response.json()["tag_name"]
    bodyinfo = response.json()["body"]
    # 使用正则
    pattern = r"\bBDS\s+(\d+\.\d+\.\d+\.\d+)\b"
    BDSVersion = re.search(pattern, bodyinfo).group(1)
    return re.search(r"\bProtocol\s+(\d+)\b", bodyinfo).group(1)

def getllcheckbagVersion():
    # 可用正则
    # versionMajor = re.search(r"\bPLUGIN_VERSION_MAJOR\s+(\d+)\b", versionInfo[10]).group(1)
    versionMajor = versionInfo[10][36:len(versionInfo[10])-1]

    versionMinor = versionInfo[11][36:len(versionInfo[11])-1]

    versionRevision = versionInfo[12][36:len(versionInfo[12])-1]

    return "v"+versionMajor+"."+versionMinor+"."+versionRevision

def getllcheckbagProtocol():
    protocolline = versionInfo[15]
    return protocolline[36:len(protocolline)-1]

def modifyVersionInfo(oldProtocol,newProtocol):
    print("Change version Info")
    #修改小版本号
    revisionline = versionInfo[12]
    versionRevision = int(revisionline[36:len(revisionline)-1])
    versionInfo[12] = revisionline.replace(str(versionRevision),str(versionRevision+1))

    #修改协议号
    protocolline = versionInfo[15]
    versionInfo[15] = protocolline.replace(oldProtocol,newProtocol)

    with open(VERSION_PATH, "r+",encoding='utf8') as file:
        file.seek(0)
        file.truncate()
        file.writelines(versionInfo)
    file.close()

    print("Change version Info Success")

def modifyChangelog():
    print("Change Changelog")
    #以下变量需要在修改后获取，否则是旧版本号
    version = getllcheckbagVersion()
    protocol = getllcheckbagProtocol()
    changlogzh = "# "+version+ "（"+protocol+"协议）\n\n- 支持"+BDSVersion + \
                 "\n- 适配LiteLoader v"+ LLVersion + \
                 "\n- 此为自动更新，可能会有问题；如有问题提交issue" + \
                 "\n- 如果LL发布了小版本，此版本不可用，请前往Action下载"
    changlogen = "# "+version+ "("+protocol+" Protocol)\n\n- Support "+BDSVersion + \
                 "\n- Support LiteLoader v"+ LLVersion +\
                 "\n- This is auto update, maybe have error. Please submit issue, If have error" +\
                 "\n- If LL released a minor version, this version is not available, please go to Action to download"
    with open('CHANGELOG.md', "r+",encoding='utf8') as filezh:
        filezh.seek(0)
        filezh.truncate()
        filezh.write(changlogzh)
    filezh.close()
    with open('CHANGELOG_en.md', "r+",encoding='utf8') as fileen:
        fileen.seek(0)
        fileen.truncate()
        fileen.write(changlogen)
    fileen.close()
    print("Change Changelog Success")

def modifyBDSLink():
    print("Change LINK.txt")
    header = {"accept":"application/vnd.github+json"}
    response = requests.get("https://api.github.com/repos/LiteLDev/LiteLoaderBDS/contents/scripts/LINK.txt",headers=header)
    linkinfo = response.json()["content"]
    link =  str(base64.b64decode(linkinfo),"utf8")
    with open('./LINK.txt', "r+") as file:
        file.seek(0)
        file.truncate()
        file.write(link)
    file.close()
    print("Change LINK.txt Success")

def commitChange():
    print("Commit Change")
    version = getllcheckbagVersion()
    repo = git.Repo("./")

    commitmsg = "Auto Support "+BDSVersion
    modify_file_list = repo.index.diff(None)
    #print([m.a_path for m in modify_file_list])
    repo.index.add([m.a_path for m in modify_file_list])
    repo.index.commit(commitmsg)
    repo.create_tag(version,"HEAD","Auto Support")
    print("Commit Change Success")

if __name__ == '__main__':
    protocol = getllcheckbagProtocol()
    llprotocol = getLiteLoaderProtocol()
    if protocol == llprotocol:
        print("Don't need Update")
    else:
        print("Need Update\nNow Start Auto Update")
        modifyVersionInfo(protocol,llprotocol)

        modifyChangelog()
        modifyBDSLink()
        commitChange()
        print("Auto modify files Success")