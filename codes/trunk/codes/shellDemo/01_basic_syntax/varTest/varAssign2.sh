 #!/bin/bash
 
 a=23              # 简单的赋值
 echo $a
 b=$a
 echo $b
 
 # 现在让我们来点小变化(命令替换).
 
 a=`echo Hello!`   # 把'echo'命令的结果传给变量'a'
 echo $a
 #使用$(...)机制来进行变量赋值(这是一种比后置引用(反引号`)更新的一种方法).
 a=$(echo Hello2!)
 echo $a
 #  注意, 如果在一个#+的命令替换结构中包含一个(!)的话, 
 #+ 那么在命令行下将无法工作.
 #+ 因为这触发了Bash的"历史机制."
 #  但是, 在脚本中使用的话, 历史功能是被禁用的, 所以就能够正常的运行.
 
 a=`ls -l`         # 把'ls -l'的结果赋值给'a'
 echo $a           # 然而, 如果没有引号的话将会删除ls结果中多余的tab和换行符.
 echo
 echo "$a"         # 如果加上引号的话, 那么就会保留ls结果中的空白符.
                   # (具体请参阅"引用"的相关章节.)
 
 echo 'over!'
 exit 0
