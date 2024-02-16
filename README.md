# chatchat

端对端加密的聊天软件

## 简介

1. 账号获取
    * Linux：
        1. 生成私钥：`openssl genpkey -algorithm RSA -out private_key.pem`
        2. 提取公钥：`openssl rsa -pubout -in private_key.pem -out public_key.pem`
    * Windows：
        1. 生成私钥：
        2. 提取公钥：

2. 登录
    * 使用 **公钥** + **私钥** 登录（每一密钥对意味着一个用户）
    * 同时绑定 **IPv4/6:port** 开启监听

3. 添加好友
    * 输入对方的 **IPv4/6:port** 与 **公钥**
    * 会直接使用对方 **IP:port** 作为 **好友 ID**
    * 当好友收到添加请求后，同意则会将对方 **公钥** 保存在本地，将自己的 **公钥** 发送给对方

4. 删除好友
    * 

3. 消息安全
    * 发送给 **好友** 的消息，使用 **好友** 的 **公钥** 加密
    * **好友** 收到消息后，使用 **好友** 的 **私钥** 解密