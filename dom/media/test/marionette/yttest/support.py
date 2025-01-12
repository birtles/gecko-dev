# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
import sys
import os
from contextlib import contextmanager

from mozproxy import get_playback
import mozinfo
import mozlog
from marionette_harness.marionette_test import MarionetteTestCase
from yttest.ytpage import using_page


mozlog.commandline.setup_logging("mozproxy", {}, {"tbpl": sys.stdout})
here = os.path.dirname(__file__)
playback_script = os.path.join(here, "playback.py")


class VideoStreamTestCase(MarionetteTestCase):
    def setUp(self):
        MarionetteTestCase.setUp(self)
        if "MOZ_UPLOAD_DIR" not in os.environ:
            os.environ["OBJ_PATH"] = "/tmp/"
        self.marionette.set_pref("media.autoplay.default", 1)

    @contextmanager
    def using_proxy(self, video_id):
        config = {}
        config["binary"] = self.marionette.bin
        config["app"] = "firefox"
        config["platform"] = mozinfo.os
        config["processor"] = mozinfo.processor
        config["run_local"] = "MOZ_UPLOAD_DIR" not in os.environ

        if "MOZ_UPLOAD_DIR" not in os.environ:
            config["obj_path"] = os.environ["OBJ_PATH"]
            playback_dir = os.path.join(config["obj_path"], "testing", "mozproxy")
        else:
            root_dir = os.path.dirname(os.path.dirname(os.environ["MOZ_UPLOAD_DIR"]))
            playback_dir = os.path.join(root_dir, "testing", "mozproxy")

        config["host"] = "localhost"
        config["playback_tool"] = "mitmproxy"
        config["playback_artifacts"] = os.path.join(here, "%s.manifest" % video_id)

        # XXX once Bug 1540622 lands, we can use the version here
        # config["playback_version"] = "4.0.4"
        # and have playback_binary_manifest default to
        # mitmproxy-rel-bin-{playback_version}-{platform}.manifest
        # so we don't have to ask amozproxy tool user to provide this:
        config[
            "playback_binary_manifest"
        ] = "mitmproxy-rel-bin-4.0.4-{platform}.manifest"

        playback_file = os.path.join(playback_dir, "%s.playback" % video_id)

        config["playback_tool_args"] = [
            "--set",
            "stream_large_bodies=30",
            "--ssl-insecure",
            "--server-replay-nopop",
            "--set",
            "upstream_cert=false",
            "-S",
            playback_file,
            "-s",
            playback_script,
            video_id,
        ]

        proxy = get_playback(config)
        if proxy is None:
            raise Exception("Could not start Proxy")
        proxy.start()
        try:
            yield proxy
        finally:
            proxy.stop()

    @contextmanager
    def youtube_video(self, video_id, **options):
        proxy = options.get("proxy", True)
        if proxy:
            with self.using_proxy(video_id):
                with using_page(video_id, self.marionette, **options) as page:
                    yield page
        else:
            with using_page(video_id, self.marionette, **options) as page:
                yield page
