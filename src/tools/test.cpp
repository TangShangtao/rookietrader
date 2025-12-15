#include "crow.h"
// 注意：根据你的环境，如果使用源码编译，可能需要 #include "crow.h"

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <cstdint> // 用于 uint16_t

// 1. 定义数据结构
struct Data {
    int a = 0;
    int b = 0;
};

// 全局变量
Data g_data;
std::mutex g_mutex;
std::unordered_set<crow::websocket::connection*> g_connections;

// 3. 广播函数
void broadcast_data() {


    crow::json::wvalue x;
    x["a"] = g_data.a;
    x["b"] = g_data.b;
    std::string message = x.dump();

    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_connections.begin();
    while (it != g_connections.end()) {
        try {
            (*it)->send_text(message);
            ++it;
        } catch (...) {
            it = g_connections.erase(it);
        }
    }
}

int main() {
    crow::SimpleApp app;
    CROW_ROUTE(app, "/")([](){
        crow::mustache::context ctx;
        return crow::mustache::load("index.html").render(ctx);
    });

    // ★★★ 修正点在下方 ★★★
    CROW_ROUTE(app, "/ws")
        .websocket(&app)
        .onopen([&](crow::websocket::connection& conn) {
            std::lock_guard<std::mutex> lock(g_mutex);
            g_connections.insert(&conn);
        })
            // 修改：增加 uint16_t code 参数
        .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t /*code*/) {
            std::lock_guard<std::mutex> lock(g_mutex);
            g_connections.erase(&conn);
            // std::cout << "Client disconnected. Reason: " << reason << " Code: " << code << std::endl;
        })
        .onmessage([&](crow::websocket::connection& /*conn*/, const std::string& /*data*/, bool /*is_binary*/) {
            // 忽略消息
        });

    std::thread server_thread([&app](){
        app.port(18080).multithreaded().run();
    });

    std::cout << "Server started on http://localhost:18080" << std::endl;
    std::cout << "Enter two integers (a b) to update data:" << std::endl;

    int in_a, in_b;
    while (std::cin >> in_a >> in_b) {
        {
//            std::lock_guard<std::mutex> lock(g_mutex);
            g_data.a = in_a;
            g_data.b = in_b;
        }
        broadcast_data();
        std::cout << "Updated: A=" << in_a << ", B=" << in_b << std::endl;
    }

    app.stop();
    server_thread.join();

    return 0;
}