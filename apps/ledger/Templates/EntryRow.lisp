(Grid Columns="1fr auto" Spacing=12 Height=44 StateClasses=(bind "DataContext/Tone")
  (StackView Spacing=0 JustifyContent="Center"
    (TextBlock Name="Memo" Text=(bind "DataContext/Memo") FontSize=16 class="ledger-ink" IgnoreHitTest=true)
    (TextBlock Text=(bind "DataContext/Date") FontSize=12 class="ledger-dim" IgnoreHitTest=true))
  (TextBlock Name="Amount" Text=(bind "DataContext/AmountText") FontSize=16 TextVerticalAlignment="Center" class="ledger-amount" IgnoreHitTest=true))
