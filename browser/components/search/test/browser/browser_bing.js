/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/ */

/*
 * Test Bing search plugin URLs
 */

"use strict";

const BROWSER_SEARCH_PREF = "browser.search.";

add_task(async function test() {
  await Services.search.init();

  let engine = Services.search.getEngineByName("Bing");
  ok(engine, "Bing");

  let base = "https://www.bing.com/search?q=foo&pc=MOZI";
  let url;

  // Test search URLs (including purposes).
  url = engine.getSubmission("foo").uri.spec;
  is(url, base + "&form=MOZSBR", "Check search URL for 'foo'");
  url = engine.getSubmission("foo", null, "contextmenu").uri.spec;
  is(url, base + "&form=MOZCON", "Check context menu search URL for 'foo'");
  url = engine.getSubmission("foo", null, "keyword").uri.spec;
  is(url, base + "&form=MOZLBR", "Check keyword search URL for 'foo'");
  url = engine.getSubmission("foo", null, "searchbar").uri.spec;
  is(url, base + "&form=MOZSBR", "Check search bar search URL for 'foo'");
  url = engine.getSubmission("foo", null, "homepage").uri.spec;
  is(url, base + "&form=MOZSPG", "Check homepage search URL for 'foo'");
  url = engine.getSubmission("foo", null, "newtab").uri.spec;
  is(url, base + "&form=MOZTSB", "Check newtab search URL for 'foo'");

  // Check search suggestion URL.
  url = engine.getSubmission("foo", "application/x-suggestions+json").uri.spec;
  is(url, "https://www.bing.com/osjson.aspx?query=foo&form=OSDJAS&language=" + getLocale(), "Check search suggestion URL for 'foo'");

  // Check all other engine properties.
  const EXPECTED_ENGINE = {
    name: "Bing",
    alias: null,
    description: "Bing. Search by Microsoft.",
    searchForm: "https://www.bing.com/search?q=&pc=MOZI&form=MOZSBR",
    hidden: false,
    wrappedJSObject: {
      queryCharset: "UTF-8",
      "_iconURL": "data:image/x-icon;base64,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",
      _urls: [
        {
          type: "application/x-suggestions+json",
          method: "GET",
          template: "https://www.bing.com/osjson.aspx",
          params: [
            {
              name: "query",
              value: "{searchTerms}",
              purpose: undefined,
            },
            {
              name: "form",
              value: "OSDJAS",
              purpose: undefined,
            },
            {
              name: "language",
              value: "{moz:locale}",
              purpose: undefined,
            },
          ],
        },
        {
          type: "text/html",
          method: "GET",
          template: "https://www.bing.com/search",
          params: [
            {
              name: "q",
              value: "{searchTerms}",
              purpose: undefined,
            },
            {
              name: "pc",
              value: "MOZI",
              purpose: undefined,
            },
            {
              name: "form",
              value: "MOZCON",
              purpose: "contextmenu",
            },
            {
              name: "form",
              value: "MOZSBR",
              purpose: "searchbar",
            },
            {
              name: "form",
              value: "MOZSPG",
              purpose: "homepage",
            },
            {
              name: "form",
              value: "MOZLBR",
              purpose: "keyword",
            },
            {
              name: "form",
              value: "MOZTSB",
              purpose: "newtab",
            },
          ],
          mozparams: {},
        },
      ],
    },
  };

  isSubObjectOf(EXPECTED_ENGINE, engine, "Bing");
});
