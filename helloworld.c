#include <furi.h>
#include <gui/gui.h>
#include <stdlib.h>
#include <gui/elements.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    FuriMutex* mutex;
} PluginState;

static void render_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    const PluginState* plugin_state = ctx;
    furi_mutex_acquire(plugin_state->mutex, FuriWaitForever);

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Hello World");

    furi_mutex_release(plugin_state->mutex);
}

int32_t hello_world_app() {
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    PluginState* plugin_state = malloc(sizeof(PluginState));

    plugin_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if (!plugin_state->mutex) {
        FURI_LOG_E("hello_world", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(plugin_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, plugin_state);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    for (bool running = true; running;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, FuriWaitForever);  // Espera indefinida para eventos

        furi_mutex_acquire(plugin_state->mutex, FuriWaitForever);

        switch (event_status) {
            case FuriStatusOk:
                switch (event.type) {
                    case EventTypeKey:
                        if (event.input.type == InputTypePress) {
                            switch (event.input.key) {
                                case InputKeyBack:
                                    printf("Back button pressed\n");
                                    running = false; // Salir de la aplicación cuando se pulsa "Back"
                                    break;
                                // Agrega más casos según sea necesario para otros botones
                            }
                        }
                        break;
                    // Agrega más casos según sea necesario para otros tipos de eventos
                }
                break;
            // Agrega más casos según sea necesario para otros estados de eventos
        }

        view_port_update(view_port);
        furi_mutex_release(plugin_state->mutex);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(plugin_state->mutex);

    return 0;
}
