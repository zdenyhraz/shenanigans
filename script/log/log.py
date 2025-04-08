import logging


class CustomFormatter(logging.Formatter):
    trace_color = "\x1b[38;2;172;136;255m"
    debug_color = "\x1b[38;2;30;144;255m"
    info_color = "\x1b[38;2;87;226;143m"
    warning_color = "\x1b[38;2;255;150;45m"
    error_color = "\x1b[38;2;255;55;55m"
    time_color = "\x1b[38;2;172;136;255m"
    reset = "\x1b[0m"
    prefix = time_color + "[%(asctime)s] "
    format_str = "%(message)s"

    FORMATS = {
        logging.DEBUG: prefix + debug_color + format_str + reset,
        logging.INFO: prefix + info_color + format_str + reset,
        logging.WARNING: prefix + warning_color + format_str + reset,
        logging.ERROR: prefix + error_color + format_str + reset,
        logging.CRITICAL: prefix + error_color + format_str + reset
    }

    def format(self, record):
        log_fmt = self.FORMATS.get(record.levelno)
        formatter = logging.Formatter(log_fmt, "%H:%M:%S")
        return formatter.format(record)


log = logging.getLogger("shenanigans")
log.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)
ch.setFormatter(CustomFormatter())
log.addHandler(ch)


def debug(message):
    log.debug(message)


def info(message):
    log.info(message)


def warning(message):
    log.warning(message)


def error(message):
    log.error(message)


if __name__ == "__main__":
    log.debug("debug h3h3h3")
    log.info("info h3h3h3")
    log.warning("warning h3h3h3")
    log.error("error h3h3h3")
