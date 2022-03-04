作用:
解析soureMap文件(*.js.map)获得源文件的位置，如通过js报错的文件行列号得到ts对应的位置

使用方法:
编译生成mapper.exe文件
```sh
mapper.exe jspath line column
eg：mapper.exe ../../mapper/test/dist/main.js 2 11
```
也可以生成dll或lib使用
生成dll的方法:
```sh
Visual Studio中打开项目,在解决方案中中右键mapper属性，修改常规属性的配置类型为"动态库(dll)"，
然后编译项目，对应的dll即生成完毕。
支持debug32、64和release32、64
```