#include "exception_handler.h"

exception_handler g_exception_handler = exception_handler();

exception_handler::exception_handler() {
    enable();
}

exception_handler::~exception_handler() {
    //disable(); 
	write_to_exception_file();
}

void exception_handler::disable()
{
    RemoveVectoredExceptionHandler(veh_handle);
}

void exception_handler::enable()
{
    veh_handle = AddVectoredExceptionHandler(1, vectored_exception_handler);
    SetUnhandledExceptionFilter(unhandled_exception_filter);
}

bool exception_handler::check_exception_handles()
{
    handlers_validity_test.caught_by_veh = false;

    __try {
        int* p = nullptr;
        *p = 42;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {


    }

	bool veh_working = handlers_validity_test.caught_by_veh.get();
    handlers_validity_test.caught_by_veh = false;

    return veh_working;
}
void exception_handler::remove_exception_handler(fn_exception_handler_callback callback)
{
    mtx.lock();

    for (auto it = exception_callbacks.begin(); it != exception_callbacks.end(); ++it) {
        if (*it == callback) {
            exception_callbacks.remove_iterator(it);
            break;
        }
    }
    mtx.release();
}
void exception_handler::remove_unhandled_exception_handler(fn_unhandled_exception_handler_callback callback)
{
    mtx.lock();

    for (auto it = unhandled_exception_callbacks.begin(); it != unhandled_exception_callbacks.end(); ++it) {
        if (*it == callback) {
            unhandled_exception_callbacks.remove_iterator(it);
            break;
        }
    }

    mtx.release();
}

void exception_handler::add_exception_handler(fn_exception_handler_callback callback) {
    mtx.lock();
    exception_callbacks.push_back(callback);
    mtx.release();
}

void exception_handler::add_unhandled_exception_handler(fn_unhandled_exception_handler_callback callback) {
    mtx.lock();
    unhandled_exception_callbacks.push_back(callback);
    mtx.release();
}

void exception_handler::on_exception(const char* str) {
    mtx.lock();
    exception_stack.push_back(str);
    mtx.release();
}

void exception_handler::write_to_exception_file(bool clear) {
    mtx.lock();
    std::ofstream file(exception_storage_file_path, std::ios::out | std::ios::binary);
    if (file.is_open()) {
        for (const auto& str : exception_stack) {
            file.write(str.c_str(), str.size());
            file.write("\n", 1);
        }
        file.close();
    }
    if (clear)
        exception_stack.clear();
    mtx.release();
}

void exception_handler::print_exceptions_on_console(bool clear) {
    mtx.lock();
    for (const auto& str : exception_stack)
        std::cout << str << std::endl;
    if (clear)
        exception_stack.clear();
    mtx.release();
}

LONG WINAPI exception_handler::vectored_exception_handler(EXCEPTION_POINTERS* exception_info) {

    switch (exception_info->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION: {
        g_exception_handler.handlers_validity_test.caught_by_veh = true;
        break;
    }
    default:
        break;
    }

    std::stringstream ss;

    ss << "[VEH] Code: " << std::hex << exception_info->ExceptionRecord->ExceptionCode
        << " Address: " << exception_info->ExceptionRecord->ExceptionAddress << "\n";

    g_exception_handler.on_exception(ss.str().c_str());

    g_exception_handler.mtx.lock();
 
    bool exception_handled = false;

    for (const auto& callback : g_exception_handler.exception_callbacks)
    {
        exception_handled = callback(exception_info);
		if (exception_handled)
			break;
    }

    g_exception_handler.mtx.release();

	return exception_handled ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI exception_handler::unhandled_exception_filter(EXCEPTION_POINTERS* exception_info) {

    std::stringstream ss;
    ss << "[UEF] Unhandled Exception! Code: " << std::hex << exception_info->ExceptionRecord->ExceptionCode
        << " Address: " << exception_info->ExceptionRecord->ExceptionAddress << "\n";
  

    g_exception_handler.on_exception(ss.str().c_str());
    g_exception_handler.write_to_exception_file();

   
    g_exception_handler.mtx.lock();

    bool exception_handled = false;

    for (const auto& callback : g_exception_handler.unhandled_exception_callbacks)
    {
        exception_handled = callback(exception_info);
        if (exception_handled)
            break;
    }

    g_exception_handler.mtx.release();

	return exception_handled ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI exception_handler::seh_exception_filter(EXCEPTION_POINTERS* exception_info) {

    std::stringstream ss;

    ss << "[SEH] Exception Caught! Code: 0x" << std::hex << exception_info->ExceptionRecord->ExceptionCode
        << " at Address: " << exception_info->ExceptionRecord->ExceptionAddress << "\n";

    std::cerr << ss.str(); 

    return EXCEPTION_EXECUTE_HANDLER;
}