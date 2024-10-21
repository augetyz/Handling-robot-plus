#!/bin/bash

project_path="$(dirname $(realpath ${BASH_SOURCE}))"
goal_file1=/home/biubiu/Desktop/shell_test/biubiu   #测试用文件输出
goal_file=/etc/rc.local                             #目标文件
use_file=/lib/systemd/system/rc-local.service       #环境配置脚本

sudo chmod +x /etc/rc.local                         #赋予执行权限

if [ "x$(grep "Install" $use_file)" != "x" ]; then  #检测开机启动脚本是否安装到执行目录中
    echo "rc-local.service文件已配置安装环境"
else
    echo "rc-local.service文件未配置安装环境"
    echo "写入配置中……"
    echo " " >> $use_file                           #向环境配置脚本添加Insatll选项
    echo "[Install]                                 
WantedBy=multi-user.target  
Alias=rc-local.service" >> $use_file
    echo "rc-local.service文件已配置OK"
fi
#rc.local需要在第一行添加脚本解释器注释，其实就是个shell脚本，估计是重定向了
if [ "x$(grep "#!/bin/bash" $goal_file)" != "x" ]; then
    echo "rc.local文件已配置启动环境"
else
    echo "rc.local文件未配置启动环境"
    echo "正在覆盖写入"
    echo "#!/bin/bash" > $goal_file
    echo "rc.local文件已配置OK"
fi
#systemd 默认读取 /etc/systemd/system 下的配置文件, 所以需要在 /etc/systemd/system 目录下创建软链接
ln -s /lib/systemd/system/rc-local.service /etc/systemd/system/ >/dev/null 2>&1

if (( $# > 0 )); then #读取参数

    echo "开机自启动文件有："
    echo $*
    for file in $@;   #挨个读取
    do
        b=${#file}
        a=`expr ${#file} - 2`
        # echo $a $b
        file_type=${file:$a:$b}  #截取字符串
        case "$file_type" in
            "sh")
                echo "$file 是shell脚本"
                file="$project_path"'/'"$file" #追加路径
                echo "绝对路径:$file"
                if test -e "$file";then        #文件存在判断
                    file="sudo .""$file"" &" #执行方式编写
                    #先判断该脚本是否已经添加到自启动
                    if [ "x$(grep "$file" $goal_file)" = "x" ]; then
                        if [ "x$(grep "exit 0" $goal_file)" != "x" ]; then  #树莓派兼容
                            sed -i "/exit 0/i\ $file" $goal_file
                        else
                            echo $file >> $goal_file
                        fi
                    else
                        echo "该脚本执行已经添加自启动选项"
                    fi
                else
                    echo "文件不存在 $file"
                fi
                
            ;;
            "py")
                echo "$file 是python脚本"
                file="$project_path"'/'"$file"               #追加路径
                echo "绝对路径:$file"
                if test -e "$file";then                      #文件存在判断
                    file="sudo /usr/bin/python3 ""$file"" &" #执行方式编写
                    #先判断该脚本是否已经添加到自启动
                    if [ "x$(grep "$file" $goal_file)" = "x" ]; then
                        if [ "x$(grep "exit 0" $goal_file)" != "x" ]; then #树莓派兼容
                            sed -i "/exit 0/i\ $file" $goal_file
                        else
                            echo $file >> $goal_file
                        fi
                    else
                        echo "该脚本执行已经添加自启动选项"
                    fi  
                else
                    echo "文件不存在 $file"
                fi
            ;;
            *)
            echo "非标准脚本，用户可自行添加"
            ;; 
        esac
    done

else
    echo "ERROR:没有参数噻"
fi

sudo systemctl enable $goal_file


