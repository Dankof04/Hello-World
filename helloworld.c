#include <furi.h>
#include <gui/gui.h>
#include <stdlib.h>
#include <gui/elements.h>

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
    PluginState* plugin_state = malloc(sizeof(PluginState));

    plugin_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if (!plugin_state->mutex) {
        FURI_LOG_E("hello_world", "cannot create mutex\r\n");
        free(plugin_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, plugin_state);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Main loop to keep the application running
    for (bool running = true; running;) {
        view_port_update(view_port);

        // Handle application exit when the "Back" button is pressed
        PluginEvent event;
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        if (event_status == FuriStatusOk) {
            if (event.type == EventTypeKey && event.input.type == InputTypePress) {
                if (event.input.key == InputKeyBack) {
                    running = false; // Salir de la aplicación cuando se pulsa "Back"
                }
            }
        }
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_mutex_free(plugin_state->mutex);

    return 0;
}
