//
// Created by Weckest on 2026-09-23.
//

#include "ModuleService.h"

#include "network/IModuleSender.h"

#include <iostream>
#include <cstring>

#include "pb_encode.h"
#include "engine/RoomEngine.h"

extern RoomEngine* g_engine;

ModuleService::ModuleService(
    IModuleSender& sender)
    : sender(sender)
{
}


void ModuleService::HandleMessage(
    const ModuleMessage& message,
    int clientSocket,
    TcpConnection& connection)
{
    switch (message.which_payload)
    {
        case ModuleMessage_heartbeat_tag:
            HandleHeartbeat(message, clientSocket, connection);
            break;

        case ModuleMessage_module_setup_request_tag:
            OnModuleConnected(message, clientSocket);
            break;

        // case ModuleMessage_button_pressed_tag:
        //     HandleButtonPressed(message, clientSocket);
        //     break;
        //
        // case ModuleMessage_button_released_tag:
        //     HandleButtonReleased(message, clientSocket);
        //     break;
        //
        // case ModuleMessage_set_led_tag:
        //     HandleSetLed(message);
        //     break;

        default:
            std::cout << "Sending message to Action Manager" << std::endl;
            g_engine->getWorld().getActionManager().handleAction(message);
            break;
    }
}

void ModuleService::OnModuleConnected(const ModuleMessage& message, int clientSocket) {
    std::cout << "module setup request" << std::endl;
    SendModuleSetup(message.device_id, clientSocket);
}

void ModuleService::HandleOutput(OutputAction &outputAction) {
    ModuleMessage output = ModuleMessage_init_zero;

    std::strcpy(output.device_id, outputAction.deviceId.c_str());

    output.which_payload = ModuleMessage_set_led_tag;
    output.payload.set_led.led_id = outputAction.outputId;
    auto& entities = g_engine->getWorld().getEntities();
    for (auto& e : entities) {
        if (!e->hasComponent<Hardware>() && !e->hasComponent<Module>()) continue;

        auto& module = e->getComponent<Module>();
        if (module.deviceId != outputAction.deviceId) continue;
        auto& hardware = e->getComponent<Hardware>();

        output.payload.set_led.state = hardware.lights.at(outputAction.outputId).state;
        sender.Send(module.tcpConnection->GetSocket(), output);
        return;
    }
}

void ModuleService::HandleHeartbeat(
    const ModuleMessage& message,
    int clientSocket,
    TcpConnection& connection)
{

    ModuleMessage response = ModuleMessage_init_zero;

    std::strncpy(
        response.device_id,
        message.device_id,
        sizeof(response.device_id) - 1
    );

    g_engine->getWorld().createModule(connection, message.device_id);

    response.which_payload = ModuleMessage_heartbeat_ack_tag;

    sender.Send(clientSocket, response);
}

void ModuleService::SendModuleSetup(const std::string &deviceId, int clientSocket) {
    ModuleMessage message = ModuleMessage_init_zero;

    message.which_payload = ModuleMessage_module_setup_tag;

    ModuleConfig* config = g_engine->getWorld().getModuleManager().getModuleConfig(deviceId);

    auto& setup = message.payload.module_setup;

    for (const auto [pin, id] : config->inputs) {
        std::cout << "Button " <<  pin << " " << id << std::endl;
        setup.input_map[setup.input_map_count++] = {pin, id};
    }

    for (const auto [pin, id] : config->outputs) {
        std::cout << "LED " <<  pin << " " << id << std::endl;
        setup.output_map[setup.output_map_count++] = {pin, id};

    }

    sender.Send(clientSocket, message);
}


