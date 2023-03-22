# hesh

***

* 特殊符号优先级参考表


    符号|优先级|作用|错误情况
    ----|:----:|---:|---
    `&`   |最低  |将程序放在后台执行|一句命令只能出现一个`&`，不能与`&&`连用
    `&&`  |最低  |等待前一个程序执行完毕执行后面一个|可以出现多个，但不能与`&`连用
    `>`   |次低  |将程序输出写入到指定文件中|无
    `>>`  |次低  |将程序输出追加到指定文件中|无
    `<`   |比`>` `>>` 高一个等级|将指定文件作为程序的输入|无
    `|`   |最高|将程序的输出作为另一个程序的输入|无

* 内置命令

    命令|作用
    ----|:---
    jobs|列出所有的作业
    fg|将后台的作业恢复到前台执行 
    sg|暂停一个作业
    quit|退出shell
    exit|退出shell
