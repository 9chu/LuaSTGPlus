const { description } = require('../../package')

function SideBarBuilder(prefix) {
  this.prefix = prefix || '';
  this.stack = [[]];
}

SideBarBuilder.prototype.page = function(link, title) {
  var last = this.stack[this.stack.length - 1];
  if (!title)
    last.push(link);
  else
    last.push({ type: 'page', link: this.perfix + link, title });
  return this;
};

SideBarBuilder.prototype.group = function(title, collapsable) {
  var last = this.stack[this.stack.length - 1];
  var children = [];
  last.push({ type: 'group', title, children, collapsable: collapsable === undefined ? true : collapsable, sidebarDepth: 2 });
  this.stack.push(children);
  return this;
};

SideBarBuilder.prototype.end = function() {
  var last = this.stack.pop();
  if (this.stack.length === 0)
    return last;
  return this;
};

module.exports = {
  /**
   * Ref：https://v1.vuepress.vuejs.org/config/#title
   */
  title: 'LuaSTGPlus',
  /**
   * Ref：https://v1.vuepress.vuejs.org/config/#description
   */
  description: description,

  /**
   * Extra tags to be injected to the page HTML `<head>`
   *
   * ref：https://v1.vuepress.vuejs.org/config/#head
   */
  head: [
    ['meta', { name: 'theme-color', content: '#3eaf7c' }],
    ['meta', { name: 'apple-mobile-web-app-capable', content: 'yes' }],
    ['meta', { name: 'apple-mobile-web-app-status-bar-style', content: 'black' }]
  ],

  /**
   * Theme configuration, here is the default theme configuration for VuePress.
   *
   * ref：https://v1.vuepress.vuejs.org/theme/default-theme-config.html
   */
  themeConfig: {
    logo: '/logo.png',
    locales: {
      '/': {
        selectText: 'Languages',
        label: '中文',
        editLinks: false,
        lastUpdated: false,
        nav: [
          { text: '指南', link: '/guide/' },
          { text: 'API', link: '/api/' },
          { text: 'GITHUB', link: 'https://github.com/9chu/LuaSTGPlus' },
        ],
        sidebar: {
          '/guide/': (new SideBarBuilder())
              .group('指南', false)
                .page('')
                .page('History')
                .page('Compile')
                .page('Cmdline')
                .page('Console')
                .group('子系统', false)
                  .page('Subsystem/Architecture')
                  .page('Subsystem/VfsSystem')
                  .page('Subsystem/AssetSystem')
                  .page('Subsystem/RenderSystem')
                  .page('Subsystem/AudioSystem')
                  .page('Subsystem/ScriptSystem')
                .end()
              .end()
            .end(),
          '/api/': (new SideBarBuilder())
              .group('API', false)
                .page('')
                .group('Legacy API', false)
                  .page('LegacyAPI/BuiltinMethods')
                .end()
                .group('Advanced API', false)
                  .page('AdvancedAPI/Overview')
                .end()
              .end()
            .end()
        }
      },
      '/i18n/en/': {
        selectText: 'Languages',
        label: 'English',
        nav: [
          { text: 'Guide', link: '/i18n/en/guide/' },
          { text: 'API', link: '/i18n/en/api/' },
          { text: 'GITHUB', link: 'https://github.com/9chu/LuaSTGPlus' },
        ]
      }
    },
  },

  locales: {
    '/': {
      lang: 'zh-CN',
      title: 'LuaSTGPlus',
      description: '小巧的跨平台弹幕游戏引擎'
    },
    '/i18n/en/': {
      lang: 'en-US',
      title: 'LuaSTGPlus',
      description: 'A Tiny Cross-Platform Danmaku Game Engine'
    }
  },

  /**
   * Apply plugins，ref：https://v1.vuepress.vuejs.org/zh/plugin/
   */
  plugins: [
    '@vuepress/plugin-back-to-top',
    '@vuepress/plugin-medium-zoom',
  ]
}
