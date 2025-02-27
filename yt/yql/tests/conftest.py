from conftest_lib.conftest_queries import *  # noqa

from yt_commands import (get, set)

from yt.environment.components.yql_agent import YqlAgent as YqlAgentComponent

from yt.environment.helpers import wait_for_dynamic_config_update

import os

import pytest

import yatest.common


def _get_artifacts_path():
    if "YDB_ARTIFACTS_PATH" in os.environ:
        return os.environ["YDB_ARTIFACTS_PATH"]
    else:
        return yatest.common.runtime.work_path("yt_binaries")


class YqlAgent():
    def __init__(self, env, count):
        self.yql_agent = YqlAgentComponent()

        self.yql_agent.prepare(env, config={
            "count": count,
            "path": yatest.common.binary_path("yt/yql/agent/bin"),
            "artifacts_path": _get_artifacts_path(),
            "native_client_supported": True,
        })

    def __enter__(self):
        self.yql_agent.run()
        self.yql_agent.wait()
        self.yql_agent.init()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.yql_agent.stop()


def update_yql_agent_environment(cls, yql_agent):
    if hasattr(cls, "YQL_AGENT_DYNAMIC_CONFIG") :
        dynconfig = getattr(cls, "YQL_AGENT_DYNAMIC_CONFIG")

        config = get("//sys/yql_agent/config")
        config["yql_agent"] = dynconfig
        set("//sys/yql_agent/config", config)

        wait_for_dynamic_config_update(yql_agent.yql_agent.client, config, "//sys/yql_agent/instances")


@pytest.fixture
def yql_agent(request):
    cls = request.cls
    count = getattr(cls, "NUM_YQL_AGENTS", 1)
    with YqlAgent(cls.Env, count) as yql_agent:
        update_yql_agent_environment(cls, yql_agent)
        yield yql_agent
