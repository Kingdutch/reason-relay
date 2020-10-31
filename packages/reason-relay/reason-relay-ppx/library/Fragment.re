open Ppxlib;
open Util;

/**
 * This constructs a module definition AST, in this case for fragments. Note it's only the definition structure,
 * not the full definition.
 */
let make =
    (
      ~loc,
      ~moduleName,
      ~refetchableQueryName,
      ~hasConnection,
      ~hasInlineDirective,
    ) => {
  let typeFromGeneratedModule = makeTypeAccessor(~loc, ~moduleName);
  let valFromGeneratedModule = makeExprAccessor(~loc, ~moduleName);
  let moduleIdentFromGeneratedModule = makeModuleIdent(~loc, ~moduleName);

  Ast_helper.Mod.mk(
    Pmod_structure([
      // The %stri PPX comes from Ppxlib and means "make a structure item AST out of this raw string"
      [%stri include [%m moduleIdentFromGeneratedModule(["Utils"])]],
      [%stri module Types = [%m moduleIdentFromGeneratedModule(["Types"])]],
      switch (refetchableQueryName) {
      | Some(queryName) => [%stri
          let useRefetchable = fRef => {
            let (fragmentData, refetchFn) =
              ReasonRelay.internal_useRefetchableFragment(
                [%e valFromGeneratedModule(["node"])],
                fRef->[%e valFromGeneratedModule(["getFragmentRef"])],
              );

            let data: [%t typeFromGeneratedModule(["Types", "fragment"])] =
              ReasonRelay.internal_useConvertedValue(
                [%e valFromGeneratedModule(["Internal", "convertFragment"])],
                fragmentData,
              );
            (
              data,
              (
                (
                  ~variables: [%t
                     makeTypeAccessor(
                       ~loc,
                       ~moduleName=queryName,
                       ["Types", "refetchVariables"],
                     )
                   ],
                  ~fetchPolicy: option(ReasonRelay.fetchPolicy),
                  ~renderPolicy: option(ReasonRelay.renderPolicy),
                  ~onComplete: option(option(Js.Exn.t) => unit),
                  (),
                ) => (
                  refetchFn(
                    variables
                    ->[%e
                        makeExprAccessor(
                          ~loc,
                          ~moduleName=queryName,
                          ["Internal", "convertVariables"],
                        )
                      ]
                    ->ReasonRelay.internal_cleanVariablesRaw
                    ->ReasonRelay.internal_cleanObjectFromUndefinedRaw,
                    ReasonRelay.internal_makeRefetchableFnOpts(
                      ~fetchPolicy?,
                      ~renderPolicy?,
                      ~onComplete?,
                      (),
                    ),
                  ): ReasonRelay.Disposable.t
                )
              ),
            );
          }
        ]
      | None =>
        %stri
        ()
      },
      [%stri
        let use = (fRef): [%t typeFromGeneratedModule(["Types", "fragment"])] => {
          let data =
            ReasonRelay.internal_useFragment(
              [%e valFromGeneratedModule(["node"])],
              fRef->[%e valFromGeneratedModule(["getFragmentRef"])],
            );

          ReasonRelay.internal_useConvertedValue(
            [%e valFromGeneratedModule(["Internal", "convertFragment"])],
            data,
          );
        }
      ],
      [%stri
        let useOpt =
            (opt_fRef)
            : option([%t typeFromGeneratedModule(["Types", "fragment"])]) => {
          let fr =
            switch (opt_fRef) {
            | Some(fRef) =>
              Some(fRef->[%e valFromGeneratedModule(["getFragmentRef"])])
            | None => None
            };

          let nullableFragmentData:
            Js.Nullable.t(
              [%t typeFromGeneratedModule(["Types", "fragment"])],
            ) =
            ReasonRelay.internal_useFragmentOpt(
              [%e valFromGeneratedModule(["node"])],
              switch (fr) {
              | Some(fr) => Some(fr)->Js.Nullable.fromOption
              | None => Js.Nullable.null
              },
            );

          let data = nullableFragmentData->Js.Nullable.toOption;

          ReasonRelay.internal_useConvertedValue(
            rawFragment =>
              switch (rawFragment) {
              | Some(rawFragment) =>
                Some(
                  rawFragment->[%e
                                 valFromGeneratedModule([
                                   "Internal",
                                   "convertFragment",
                                 ])
                               ],
                )
              | None => None
              },
            data,
          );
        }
      ],
      hasInlineDirective
        ? [%stri
          let readInline =
              (fRef): [%t typeFromGeneratedModule(["Types", "fragment"])] => {
            ReasonRelay.internal_readInlineData(
              [%e valFromGeneratedModule(["node"])],
              fRef->[%e valFromGeneratedModule(["getFragmentRef"])],
            )
            ->[%e valFromGeneratedModule(["Internal", "convertFragment"])];
          }
        ]
        : [%stri ()],
      switch (hasConnection, refetchableQueryName) {
      | (true, Some(queryName)) => [%stri
          let usePagination =
              (fr)
              : ReasonRelay.paginationFragmentReturn(
                  [%t typeFromGeneratedModule(["Types", "fragment"])],
                  [%t
                    makeTypeAccessor(
                      ~loc,
                      ~moduleName=queryName,
                      ["Types", "refetchVariables"],
                    )
                  ],
                ) => {
            let p =
              ReasonRelay.internal_usePaginationFragment(
                [%e valFromGeneratedModule(["node"])],
                [%e valFromGeneratedModule(["getFragmentRef"])](fr),
              );
            let data =
              ReasonRelay.internal_useConvertedValue(
                [%e valFromGeneratedModule(["Internal", "convertFragment"])],
                p.data,
              );

            {
              data,
              loadNext: (~count, ~onComplete=?, ()) =>
                p.loadNext(.
                  count,
                  {
                    onComplete:
                      onComplete->ReasonRelay.internal_nullableToOptionalExnHandler,
                  },
                ),
              loadPrevious: (~count, ~onComplete=?, ()) =>
                p.loadPrevious(.
                  count,
                  {
                    onComplete:
                      onComplete->ReasonRelay.internal_nullableToOptionalExnHandler,
                  },
                ),
              hasNext: p.hasNext,
              hasPrevious: p.hasPrevious,
              isLoadingNext: p.isLoadingNext,
              isLoadingPrevious: p.isLoadingPrevious,
              refetch:
                (
                  ~variables: [%t
                     makeTypeAccessor(
                       ~loc,
                       ~moduleName=queryName,
                       ["Types", "refetchVariables"],
                     )
                   ],
                  ~fetchPolicy=?,
                  ~renderPolicy=?,
                  ~onComplete=?,
                  (),
                ) =>
                p.refetch(.
                  variables
                  ->[%e
                      makeExprAccessor(
                        ~loc,
                        ~moduleName=queryName,
                        ["Internal", "convertVariables"],
                      )
                    ]
                  ->ReasonRelay.internal_cleanVariablesRaw
                  ->ReasonRelay.internal_cleanObjectFromUndefinedRaw,
                  ReasonRelay.internal_makeRefetchableFnOpts(
                    ~onComplete?,
                    ~fetchPolicy?,
                    ~renderPolicy?,
                    (),
                  ),
                ),
            };
          }
        ]
      | _ =>
        %stri
        ()
      },
      switch (hasConnection, refetchableQueryName) {
      | (true, Some(queryName)) => [%stri
          let useBlockingPagination =
              (fRef)
              : ReasonRelay.paginationBlockingFragmentReturn(
                  [%t typeFromGeneratedModule(["Types", "fragment"])],
                  [%t
                    makeTypeAccessor(
                      ~loc,
                      ~moduleName=queryName,
                      ["Types", "refetchVariables"],
                    )
                  ],
                ) => {
            let p =
              ReasonRelay.internal_useBlockingPaginationFragment(
                [%e valFromGeneratedModule(["node"])],
                [%e valFromGeneratedModule(["getFragmentRef"])](fRef),
              );
            let data =
              ReasonRelay.internal_useConvertedValue(
                [%e valFromGeneratedModule(["Internal", "convertFragment"])],
                p.data,
              );

            {
              data,
              loadNext: (~count, ~onComplete=?, ()) =>
                p.loadNext(.
                  count,
                  {
                    onComplete:
                      onComplete->ReasonRelay.internal_nullableToOptionalExnHandler,
                  },
                ),
              loadPrevious: (~count, ~onComplete=?, ()) =>
                p.loadPrevious(.
                  count,
                  {
                    onComplete:
                      onComplete->ReasonRelay.internal_nullableToOptionalExnHandler,
                  },
                ),
              hasNext: p.hasNext,
              hasPrevious: p.hasPrevious,
              refetch:
                (
                  ~variables: [%t
                     makeTypeAccessor(
                       ~loc,
                       ~moduleName=queryName,
                       ["Types", "refetchVariables"],
                     )
                   ],
                  ~fetchPolicy=?,
                  ~renderPolicy=?,
                  ~onComplete=?,
                  (),
                ) =>
                p.refetch(.
                  variables
                  ->[%e
                      makeExprAccessor(
                        ~loc,
                        ~moduleName=queryName,
                        ["Internal", "convertVariables"],
                      )
                    ]
                  ->ReasonRelay.internal_cleanVariablesRaw
                  ->ReasonRelay.internal_cleanObjectFromUndefinedRaw,
                  ReasonRelay.internal_makeRefetchableFnOpts(
                    ~onComplete?,
                    ~fetchPolicy?,
                    ~renderPolicy?,
                    (),
                  ),
                ),
            };
          }
        ]
      | _ =>
        %stri
        ()
      },
      switch (refetchableQueryName, hasConnection) {
      | (Some(queryName), _) => [%stri
          let makeRefetchVariables = [%e
            makeExprAccessor(
              ~loc,
              ~moduleName=queryName,
              ["Types", "makeRefetchVariables"],
            )
          ]
        ]
      | _ =>
        %stri
        ()
      },
    ]),
  );
};
