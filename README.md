# Transaction Cost Analysis (TCA) Toolkit

A professional-grade C++ implementation of Transaction Cost Analysis tools for quantitative trading firms. This toolkit provides comprehensive analysis of trading costs, market impact modeling, and execution schedule optimization.

## Overview

This project is a C++ Transaction Cost Analysis (TCA) toolkit designed for quantitative trading. It offers:

- Implementation Shortfall (IS) analysis for measuring hidden trading costs
- Market impact parameter calibration (e.g., temporary impact slope η)
- Execution schedule optimization balancing cost and risk
- Participation-of-volume (POV) constraints handling
- JSON/CSV reporting capabilities

## Implementation Breakdown

The toolkit is organized into several core components:

1. **Implementation Shortfall (IS) Analysis**
   - Calculates realized trading costs
   - Measures price impact and timing costs
   - Handles both explicit (fees) and implicit costs

2. **Market Impact Modeling**
   - Calibrates temporary and permanent impact parameters
   - Estimates price impact as a function of order size
   - Accounts for market volatility and liquidity

3. **Execution Optimization**
   - Generates optimal trading schedules
   - Balances execution costs against market risk
   - Respects user-defined POV constraints
   - Implements efficient numerical optimization

4. **Reporting Engine**
   - Produces detailed JSON/CSV reports
   - Includes trade analytics and performance metrics
   - Supports both historical analysis and forward-looking optimization

## Source Code Hierarchy

```
├── include/tca/          # Header files
│   ├── Impact.hpp       # Market impact models
│   ├── IO.hpp          # Input/Output operations
│   ├── IS.hpp          # Implementation Shortfall analysis
│   ├── Market.hpp      # Market data structures
│   ├── Optimize.hpp    # Execution optimization
│   ├── Report.hpp      # Report generation
│   ├── Types.hpp       # Common data types
│   └── utils.hpp       # Utility functions
├── src/                 # Implementation files
│   ├── Impact.cpp
│   ├── IO.cpp
│   ├── IS.cpp
│   ├── Optimize.cpp
│   └── Report.cpp
├── tools/               # Command-line tools
│   └── tca.cpp         # Main CLI interface
└── build/              # Compiled binaries and objects
```

## Prerequisites

- Modern C++ compiler supporting C++17 or later
- CMake 3.10 or higher
- nlohmann/json library for JSON processing
- Basic linear algebra library (built-in)

## Building the Project

```bash
make clean    # Clean previous builds
make         # Build the project
```

## Usage

The toolkit provides a unified command-line interface through the `tca` executable:

1. Analyze historical fills:
```bash
./build/tca analyze --fills data/fills.csv --market data/mkt.csv
```

2. Calibrate impact model:
```bash
./build/tca calibrate --impact data/impact.json
```

3. Optimize execution:
```bash
./build/tca optimize --order data/order.json
```

## Input Data Formats

The toolkit accepts various input formats:

- **fills.csv**: Historical execution data
- **mkt.csv**: Market data including prices, volumes, and volatility
- **order.json**: Order specifications for optimization
- **impact.json**: Impact model parameters

## Output Reports

Reports are generated in both JSON and CSV formats:
- Transaction cost breakdown
- Market impact analysis
- Optimized execution schedules
- Performance metrics

## License

[Add your license information here]

## Contributing

[Add contribution guidelines here]

## Authors

[Add author information here]
