import logging

# https://stackoverflow.com/questions/384076/how-can-i-color-python-logging-output/56944256#56944256


class CustomFormatter(logging.Formatter):
    grey = "\x1b[38;20m"
    blue = '\033[94m'
    yellow = "\x1b[33;20m"
    red = "\x1b[31;20m"
    green = '\033[32m'
    bold_red = "\x1b[31;1m"
    reset = "\x1b[0m"
    # format = "%(asctime)s - %(name)s - %(levelname)s - %(message)s (%(filename)s:%(lineno)d)"
    format = "[%(asctime)s] %(message)s"
    format_file = "[%(asctime)s] %(message)s (%(filename)s:%(lineno)d)"

    FORMATS = {
        logging.DEBUG: blue + format + reset,
        logging.INFO: green + format + reset,
        logging.WARNING: yellow + format + reset,
        logging.ERROR: bold_red + format_file + reset,
        logging.CRITICAL: bold_red + format_file + reset
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
    log.debug("debug ahojky broo")
    log.info("info ahojky broo")
    log.warning("warning ahojky broo")
    log.error("error ahojky broo")
