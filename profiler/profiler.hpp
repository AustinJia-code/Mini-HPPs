/**
 * @file profiler.hpp
 * @author Austin Jia
 * @brief Manual stack profiler with flame graph output.
 * @namespace prof
 *
 * @features
 *      - Manual start/stop and RAII scope guards
 *      - Self-contained HTML flame graph output (vibe-coded :D)
 *
 * @details
 *      - Times recorded in nanoseconds
 *      - Nested calls tracked as a call tree
 */

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace prof
{

using ns_t = int64_t;

/**
 * Internal details, not part of the public API.
 */
namespace detail
{
    using steady_clock = std::chrono::steady_clock;

    inline ns_t get_time_ns ()
    {
        auto now = steady_clock::now ();
        return ns_t {std::chrono::duration_cast<std::chrono::nanoseconds>
                     (now.time_since_epoch ()).count ()};
    }

    inline std::string ns_to_str (ns_t ns)
    {
        if (ns < 1000)
            return std::to_string (ns) + "ns";
        
        if (ns < 1000000)
            return std::to_string (ns / 1000) + "us";
        
        if (ns < 1000000000)
            return std::to_string (ns / 1000000) + "ms";
        
        return std::to_string (ns / 1000000000) + "s";
    }
}
namespace d = detail;

/**
 * A single timed call in the profiled call tree.
 */
struct Node
{
    std::string name;
    ns_t start;
    ns_t end = 0;
    int parent = -1;
    std::vector<int> children {};
};

/**
 * Collects timed call tree data and renders it as an HTML flame graph.
 */
class Profiler
{
private:
    std::vector<Node> nodes_ {};
    std::vector<int> stack_ {};

    int add_node (const std::string& name, int parent)
    {
        int idx = (int) nodes_.size ();

        nodes_.push_back
        (
            Node
            {
                name,
                d::get_time_ns (),
                0,
                parent
            }
        );

        return idx;
    }

    int max_depth (int idx, int depth) const
    {
        int m = depth;
        for (int child : nodes_[idx].children)
            m = std::max (m, max_depth (child, depth + 1));
        return m;
    }

    void write_node_svg (std::ostringstream& out, int idx, int depth,
                         ns_t origin, ns_t total,
                         double width, double row_h) const
    {
        static const char* colors[] =
        {
            "#e06c75","#e5c07b","#61afef","#98c379",
            "#c678dd","#56b6c2","#be5046","#d19a66"
        };

        const Node& node = nodes_[idx];
        ns_t dur = node.end - node.start;

        double x = (double)(node.start - origin) / total * width;
        double w = (double) dur / total * width;
        double y = depth * row_h;
        double bar_w = std::max (w - 1.0, 0.0);

        std::string label = node.name + " (" + d::ns_to_str (dur) + ")";
        const char* fill  = colors[depth % 8];

        out << "<g>"
            << "<rect x=\""      << x     << "\""
            << " y=\""           << y     << "\""
            << " width=\""       << bar_w << "\""
            << " height=\""      << (row_h - 2) << "\""
            << " fill=\""        << fill  << "\""
            << " rx=\"2\""
            << " data-label=\""  << label << "\">"
            << "<title>"         << label << "</title>"
            << "</rect>";

        if (bar_w > 20)
        {
            out << "<clipPath id=\"c" << idx << "\">"
                << "<rect x=\"" << x << "\" y=\"" << y
                << "\" width=\"" << bar_w << "\" height=\"" << row_h << "\"/>"
                << "</clipPath>"
                << "<text"
                << " x=\""           << (x + 4)          << "\""
                << " y=\""           << (y + row_h * 0.65) << "\""
                << " font-size=\"12\" fill=\"#fff\""
                << " clip-path=\"url(#c" << idx << ")\">"
                << node.name
                << "</text>";
        }

        out << "</g>\n";

        for (int child : node.children)
            write_node_svg (out, child, depth + 1, origin, total, width, row_h);
    }

public:
    /**
     * Begin a named timing scope.
     * Must be paired with stop().
     *
     * @param name Label for this scope in the flame graph
     */
    void start (const std::string& name)
    {
        int parent = stack_.empty () ? -1 : stack_.back ();
        int idx = add_node (name, parent);

        if (parent >= 0)
            nodes_[parent].children.push_back (idx);

        stack_.push_back (idx);
    }

    /**
     * End the innermost active scope.
     */
    void stop ()
    {
        if (stack_.empty ())
            return;

        nodes_[stack_.back ()].end = d::get_time_ns ();
        stack_.pop_back ();
    }

    /**
     * Clear all recorded data.
     */
    void reset ()
    {
        nodes_.clear ();
        stack_.clear ();
    }

    /**
     * Output the call tree to an HTML flame graph.
     *
     * @return HTML string
     */
    std::string to_html () const
    {
        if (nodes_.empty ())
            return "<!DOCTYPE html><html><body>No profiling data.</body></html>";

        // Collect root nodes and overall time range
        std::vector<int> roots;
        for (int i = 0; i < (int) nodes_.size (); ++i)
            if (nodes_[i].parent < 0)
                roots.push_back (i);

        ns_t origin = nodes_[roots[0]].start;
        ns_t end = 0;
        for (int r : roots)
        {
            origin = std::min (origin, nodes_[r].start);
            end    = std::max (end, nodes_[r].end);
        }
        ns_t total = std::max (end - origin, ns_t {1});

        // SVG dimensions
        const double width = 1200.0;
        const double row_h = 26.0;

        int depth = 0;
        for (int r : roots)
            depth = std::max (depth, max_depth (r, 0));

        double height = (depth + 1) * row_h;

        // Build SVG
        std::ostringstream svg;
        svg << std::fixed;
        svg << "<svg xmlns=\"http://www.w3.org/2000/svg\""
            << " width=\""  << width  << "\""
            << " height=\"" << height << "\""
            << " style=\"font-family:monospace;display:block\">\n";

        for (int r : roots)
            write_node_svg (svg, r, 0, origin, total, width, row_h);

        svg << "</svg>\n";

        // Wrap in HTML with a hover tooltip
        std::ostringstream html;
        html << "<!DOCTYPE html>\n<html>\n<head>\n"
             << "<meta charset=\"utf-8\">\n"
             << "<title>Flame Graph</title>\n"
             << "<style>\n"
             << "  body { margin:0; background:#1e1e2e; color:#cdd6f4; font-family:monospace }\n"
             << "  #tip { position:fixed; background:#313244; color:#cdd6f4;"
             << " padding:5px 10px; border-radius:4px; font-size:13px;"
             << " pointer-events:none; display:none; white-space:nowrap }\n"
             << "</style>\n"
             << "</head>\n<body>\n"
             << "<div id=\"tip\"></div>\n"
             << svg.str ()
             << "<script>\n"
             << "const tip = document.getElementById('tip');\n"
             << "document.querySelectorAll('rect[data-label]').forEach(r => {\n"
             << "  r.addEventListener('mousemove', e => {\n"
             << "    tip.textContent = r.dataset.label;\n"
             << "    tip.style.display = 'block';\n"
             << "    tip.style.left = (e.clientX + 14) + 'px';\n"
             << "    tip.style.top  = (e.clientY + 14) + 'px';\n"
             << "  });\n"
             << "  r.addEventListener('mouseleave', () => tip.style.display = 'none');\n"
             << "});\n"
             << "</script>\n"
             << "</body>\n</html>\n";

        return html.str ();
    }

    /**
     * Write the HTML flame graph to a file.
     *
     * @param path Output file path
     */
    void save_html (const std::string& path) const
    {
        std::ofstream f (path);
        f << to_html ();
    }
};

/**
 * RAII scope guard, calls profiler.stop() on destruction.
 */
struct Scope
{
    Profiler& profiler;

    Scope (Profiler& p, const std::string& name)
        : profiler (p)
    {
        profiler.start (name);
    }

    ~Scope ()
    {
        profiler.stop ();
    }

    Scope (const Scope&) = delete;
    Scope& operator= (const Scope&) = delete;
};

}

/**
 * Convenience macro for a scoped profile region.
 * Usage: PROF_SCOPE(my_profiler, "label")
 */
#define PROF_SCOPE(profiler, name) \
    prof::Scope _prof_scope_##__LINE__ (profiler, name)
